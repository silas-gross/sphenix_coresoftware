/*!
 * \file MvtxCombinedRawDataDecoder.cc
 * \author Cameron Dean <cameron.dean@cern.ch>
 * \author Jakub Kvapil <jakub.kvapil@cern.ch>
 */

#include "MvtxCombinedRawDataDecoder.h"

#include <trackbase/MvtxDefs.h>
#include <trackbase/MvtxEventInfov2.h>
#include <trackbase/TrkrHitSet.h>
#include <trackbase/TrkrHitSetContainerv1.h>
#include <trackbase/TrkrHitv2.h>

#include <fun4all/Fun4AllServer.h>

#include <ffarawobjects/Gl1RawHit.h>
#include <ffarawobjects/MvtxRawEvtHeader.h>
#include <ffarawobjects/MvtxRawHit.h>
#include <ffarawobjects/MvtxRawHitContainer.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/getClass.h>
#include <phool/recoConsts.h>

#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>  // for accessing the MVTX hot pixel file from the CDB

#include <algorithm>
#include <array>
#include <cassert>

//_________________________________________________________
MvtxCombinedRawDataDecoder::MvtxCombinedRawDataDecoder(const std::string &name)
  : SubsysReco(name)
{
}

//_____________________________________________________________________
int MvtxCombinedRawDataDecoder::Init(PHCompositeNode * /*topNode*/)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MvtxCombinedRawDataDecoder::InitRun(PHCompositeNode *topNode)
{
  // get dst node
  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode =
      dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cout << "MvtxCombinedRawDataDecoder::InitRun - DST Node missing, "
                 "doing nothing."
              << std::endl;
    exit(1);
  }

  // create hitset container if needed
  hit_set_container =
      findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
  if (!hit_set_container)
  {
    // find or create TRKR node
    auto trkrNode = dynamic_cast<PHCompositeNode *>(
        iter.findFirst("PHCompositeNode", "TRKR"));
    if (!trkrNode)
    {
      trkrNode = new PHCompositeNode("TRKR");
      dstNode->addNode(trkrNode);
    }

    // create container and add to the tree
    hit_set_container = new TrkrHitSetContainerv1;
    auto newNode = new PHIODataNode<PHObject>(hit_set_container, "TRKR_HITSET",
                                              "PHObject");
    trkrNode->addNode(newNode);
  }

  // Check if MVTX event header already exists
  if (m_writeMvtxEventHeader)
  {
    auto mvtxNode = dynamic_cast<PHCompositeNode *>(
        iter.findFirst("PHCompositeNode", "MVTX"));
    if (!mvtxNode)
    {
      mvtxNode = new PHCompositeNode("MVTX");
      dstNode->addNode(mvtxNode);
    }

    mvtx_event_header =
        findNode::getClass<MvtxEventInfo>(mvtxNode, "MVTXEVENTHEADER");
    if (!mvtx_event_header)
    {
      mvtx_event_header = new MvtxEventInfov2();
      auto newHeader = new PHIODataNode<PHObject>(
          mvtx_event_header, "MVTXEVENTHEADER", "PHObject");
      mvtxNode->addNode(newHeader);
    }
  }

  mvtx_raw_event_header =
      findNode::getClass<MvtxRawEvtHeader>(topNode, m_MvtxRawEvtHeaderNodeName);

  Fun4AllServer *se = Fun4AllServer::instance();

  if (!mvtx_raw_event_header)
  {
    se->unregisterSubsystem(this);
  }

  getStrobeLength();

  // Mask Hot MVTX Pixels
  std::string database = CDBInterface::instance()->getUrl(
      "MVTX_HotPixelMap");  // This is specifically for MVTX Hot Pixels
  CDBTTree *cdbttree = new CDBTTree(database);
  int NPixel = -1;
  NPixel = cdbttree->GetSingleIntValue("TotalHotPixels");

  for (int i = 0; i < NPixel; i++)
  {
    int Layer = cdbttree->GetIntValue(i, "layer");
    int Stave = cdbttree->GetIntValue(i, "stave");
    int Chip = cdbttree->GetIntValue(i, "chip");
    int Col = cdbttree->GetIntValue(i, "col");
    int Row = cdbttree->GetIntValue(i, "row");

    TrkrDefs::hitsetkey HotPixelHitKey =
        MvtxDefs::genHitSetKey(Layer, Stave, Chip, 0);
    TrkrDefs::hitkey HotHitKey = MvtxDefs::genHitKey(Col, Row);
    m_hotPixelMap.push_back({std::make_pair(HotPixelHitKey, HotHitKey)});
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

//___________________________________________________________________________
int MvtxCombinedRawDataDecoder::process_event(PHCompositeNode *topNode)
{
  mvtx_raw_event_header =
      findNode::getClass<MvtxRawEvtHeader>(topNode, m_MvtxRawEvtHeaderNodeName);
  if (Verbosity() >= 3)
  {
    mvtx_raw_event_header->identify();
  }

  hit_set_container =
      findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");

  mvtx_hit_container =
      findNode::getClass<MvtxRawHitContainer>(topNode, m_MvtxRawHitNodeName);

  if (!mvtx_hit_container)
  {
    std::cout << PHWHERE << "::" << __func__ << ": Could not get \""
              << m_MvtxRawHitNodeName << "\" from Node Tree" << std::endl;
    std::cout << "Have you built this yet?" << std::endl;
    exit(1);
  }

  auto gl1 = findNode::getClass<Gl1RawHit>(topNode, "GL1RAWHIT");
  if (!gl1 && (Verbosity() >= 4))
  {
    std::cout << PHWHERE << "Could not get gl1 raw hit" << std::endl;
  }
  //Could we just get the first strobe BCO instead of setting this to 0?
  //Possible problem, what if the first BCO isn't the mean, then we'll shift tracker hit sets? Probably not a bad thing but depends on hit stripping
  uint64_t gl1rawhitbco = gl1 ? gl1->get_bco() : 0;
  // get the last 40 bits by bit shifting left then right to match
  // to the mvtx bco
  auto lbshift = gl1rawhitbco << 24U;
  auto gl1bco = lbshift >> 24U;

  if (Verbosity() >= 3)
  {
    mvtx_hit_container->identify();
  }

  uint64_t strobe = -1;  // Initialise to -1 for debugging
  uint8_t layer = 0;
  uint8_t stave = 0;
  uint8_t chip = 0;
  uint16_t row = 0;
  uint16_t col = 0;
  std::vector<std::pair<uint64_t, uint32_t> > strobe_bc_pairs;
  std::set<uint64_t> l1BCOs = mvtx_raw_event_header->getMvtxLvL1BCO();
  auto mvtxbco = *l1BCOs.begin();
  if (gl1 && (Verbosity() > 2))
  {
    std::cout << "MVTX header BCO " << mvtxbco << " and GL1 BCO " << gl1bco
              << std::endl;
  }

  if (m_writeMvtxEventHeader)
  {
    mvtx_event_header =
        findNode::getClass<MvtxEventInfo>(topNode, "MVTXEVENTHEADER");
    assert(mvtx_event_header);
  }

  for (unsigned int i = 0; i < mvtx_hit_container->get_nhits(); i++)
  {
    mvtx_hit = mvtx_hit_container->get_hit(i);
    strobe = mvtx_hit->get_bco();
    layer = mvtx_hit->get_layer_id();
    stave = mvtx_hit->get_stave_id();
    chip = mvtx_hit->get_chip_id();
    row = mvtx_hit->get_row();
    col = mvtx_hit->get_col();

    uint64_t bcodiff = gl1 ? gl1bco - strobe : 0;
    double timeElapsed = bcodiff * 0.106;  // 106 ns rhic clock
    int index = std::floor(timeElapsed / m_strobeWidth);

    if (Verbosity() >= 10)
    {
      mvtx_hit->identify();
    }

    const TrkrDefs::hitsetkey hitsetkey =
        MvtxDefs::genHitSetKey(layer, stave, chip, index);
    if (!hitsetkey)
    {
      continue;
    }

    // get matching hitset
    const auto hitset_it = hit_set_container->findOrAddHitSet(hitsetkey);

    // generate hit key
    const TrkrDefs::hitkey hitkey = MvtxDefs::genHitKey(col, row);

    // find existing hit, or create
    auto hit = hitset_it->second->getHit(hitkey);
    if (hit)
    {
      if(Verbosity() > 1)
	{
      std::cout << PHWHERE << "::" << __func__
                << " - duplicated hit, hitsetkey: " << hitsetkey
                << " hitkey: " << hitkey << std::endl;
	}
      continue;
    }

    const TrkrDefs::hitsetkey hitsetkeymask =
        MvtxDefs::genHitSetKey(layer, stave, chip, 0);

    if (std::find(m_hotPixelMap.begin(), m_hotPixelMap.end(),
                  std::make_pair(hitsetkeymask, hitkey)) ==
        m_hotPixelMap.end())
    {
      // create hit and insert in hitset
      hit = new TrkrHitv2;
      hitset_it->second->addHitSpecificKey(hitkey, hit);
    }
  }

  mvtx_event_header->set_strobe_BCO(strobe);
  if (m_writeMvtxEventHeader)
  {
    for (auto &iter : l1BCOs)
    {
      mvtx_event_header->set_strobe_BCO_L1_BCO(strobe, iter);
    }
    if (Verbosity() >= 2)
    {
      mvtx_event_header->identify();
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

//_____________________________________________________________________
int MvtxCombinedRawDataDecoder::End(PHCompositeNode * /*topNode*/)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void MvtxCombinedRawDataDecoder::removeDuplicates(
    std::vector<std::pair<uint64_t, uint32_t> > &v)
{
  auto end = v.end();
  for (auto it = v.begin(); it != end; ++it)
  {
    end = remove(it + 1, end, *it);
  }
  v.erase(end, v.end());
}

void MvtxCombinedRawDataDecoder::getStrobeLength()
{
  recoConsts *rc = recoConsts::instance();
  int m_runNumber = rc->get_IntFlag("RUNNUMBER");

  std::string executable_command = "psql -h sphnxdaqdbreplica daq --csv -c \"SELECT strobe FROM mvtx_strobe WHERE hostname = \'mvtx0\' AND runnumber = ";
  executable_command += std::to_string(m_runNumber);
  executable_command += ";\" | tail -n 1";

  std::string strobe_query = exec(executable_command.c_str());

  try
  {
    m_strobeWidth = stof(strobe_query);
  }
  catch (std::invalid_argument const& ex)
  {
    if (Verbosity() >= 1)
    {
      std::cout << PHWHERE << ":: Run number " << m_runNumber << " has no strobe length in the DAQ database, using " << m_strobeWidth << " microseconds" << std::endl;
    }
  }
}

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string MvtxCombinedRawDataDecoder::exec(const char *cmd)
{
  std::array<char, 128> buffer = {};
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }
  return result;
}
