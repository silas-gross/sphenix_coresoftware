// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef TRACKQA_H
#define TRACKQA_H

#include <fun4all/SubsysReco.h>
#include <trackbase/ActsGeometry.h>
#include <trackbase/TrkrDefs.h>

#include <string>
#include <vector>

class SvtxTrack;
class PHCompositeNode;

class TrackQA : public SubsysReco
{
 public:
  TrackQA(const std::string &name = "TrackQA");

  ~TrackQA() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int EndRun(const int runnumber) override;
  int End(PHCompositeNode *topNode) override;

  void beginRun(const int run) { m_beginRun = run; }
  void endRun(const int run) { m_endRun = run; }

 private:
  std::vector<TrkrDefs::cluskey> get_cluster_keys(SvtxTrack *track);
  void createHistos();
  // xy slope, xy int, zr slope, zr int
  std::tuple<float, float, float, float> lineFitClusters(std::vector<Acts::Vector3> &positions) const;
  std::string getHistoPrefix() const;

  int m_event = 0;
  int m_vertices = 0;
  int m_tracks = 0;
  std::string m_trackMapName = "SvtxTrackMap";
  std::string m_vertexMapName = "SvtxVertexMap";
  int m_beginRun = 25900;
  int m_endRun = 26200;
  int m_runbins = m_endRun - m_beginRun;
};

#endif  // TRACKQA_H
