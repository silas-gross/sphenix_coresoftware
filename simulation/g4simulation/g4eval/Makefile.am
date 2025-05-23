##############################################
# please use alphabetical order for everything, it's just unreadable for others when in random order

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -isystem$(OFFLINE_MAIN)/include \
  -isystem`root-config --incdir`

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

lib_LTLIBRARIES = \
   libg4eval_io.la \
   libg4eval.la

libg4eval_io_la_LIBADD = \
  -lphool

libg4eval_la_LIBADD = \
  libg4eval_io.la \
  -lcalo_io \
  -lCLHEP \
  -lfun4all \
  -lg4detectors_io \
  -ljetbase \
  -lglobalvertex_io \
  -lg4tracking_io \
  -lintt_io \
  -lmicromegas_io \
  -lmvtx_io \
  -lphhepmc_io \
  -lphg4hit \
  -ltrack \
  -ltrackbase_historic \
  -ltpc_io

pkginclude_HEADERS = \
  BaseTruthEval.h \
  CaloEvalStack.h \
  CaloEvaluator.h \
  CaloRawClusterEval.h \
  CaloRawTowerEval.h \
  CaloTruthEval.h \
  DSTCompressor.h \
  DSTEmulator.h \
  EventEvaluator.h \
  FillTruthRecoMatchMap.h \
  FillClusMatchTree.h \
  JetEvalStack.h \
  JetEvaluator.h \
  JetRecoEval.h \
  JetTruthEval.h \
  MomentumEvaluator.h \
  PHG4DSTReader.h \
  PHG4DstCompressReco.h \
  SvtxClusterEval.h \
  SvtxEvalStack.h \
  SvtxEvaluator.h \
  SvtxHitEval.h \
  SvtxTrackEval.h \
  SvtxTruthEval.h \
  SvtxTruthRecoTableEval.h \
  SvtxVertexEval.h \
  g4evalfn.h \
  ClusKeyIter.h \
  TrackClusEvaluator.h \
  TrkrClusterIsMatcher.h \
  TrkrClusLoc.h \
  TrackEvaluation.h \
  TrackEvaluationContainer.h \
  TrackEvaluationContainerv1.h  \
  TruthRecoTrackMatching.h \
  compressor_generator.h \
  DSTTrackInfoWriter.h \
  DSTTrackInfoReader.h


ROOTDICTS = \
  TrackEvaluationContainer_Dict.cc \
  TrackEvaluationContainerv1_Dict.cc

pcmdir = $(libdir)
nobase_dist_pcm_DATA = \
  TrackEvaluationContainer_Dict_rdict.pcm \
  TrackEvaluationContainerv1_Dict_rdict.pcm

libg4eval_io_la_SOURCES = \
  $(ROOTDICTS) \
  TrackEvaluationContainerv1.cc

libg4eval_la_SOURCES = \
  BaseTruthEval.cc \
  CaloEvalStack.cc \
  CaloEvaluator.cc \
  CaloRawClusterEval.cc \
  CaloRawTowerEval.cc \
  CaloTruthEval.cc \
  DSTEmulator.cc \
  EventEvaluator.cc \
  FillTruthRecoMatchMap.cc \
  FillClusMatchTree.cc \
  JetEvalStack.cc \
  JetEvaluator.cc \
  JetRecoEval.cc \
  JetTruthEval.cc \
  MomentumEvaluator.cc \
  PHG4DSTReader.cc \
  PHG4DstCompressReco.cc \
  SvtxClusterEval.cc \
  SvtxEvalStack.cc \
  SvtxEvaluator.cc \
  SvtxHitEval.cc \
  SvtxTrackEval.cc \
  SvtxTruthEval.cc \
  SvtxTruthRecoTableEval.cc \
  SvtxVertexEval.cc \
  g4evalfn.cc \
  ClusKeyIter.cc \
  TrackClusEvaluator.cc \
  TrkrClusterIsMatcher.cc \
  TrackEvaluation.cc \
  TruthRecoTrackMatching.cc \
  DSTTrackInfoWriter.cc \
  DSTTrackInfoReader.cc

# Rule for generating table CINT dictionaries.
%_Dict.cc: %.h %LinkDef.h
	rootcint -f $@ @CINTDEFS@ $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

#just to get the dependency
%_Dict_rdict.pcm: %_Dict.cc ;

################################################
# linking tests

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals_g4eval_io \
  testexternals_g4eval

testexternals_g4eval_io_SOURCES = testexternals.cc
testexternals_g4eval_io_LDADD = libg4eval_io.la

testexternals_g4eval_SOURCES = testexternals.cc
testexternals_g4eval_LDADD = libg4eval.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f *Dict* $(BUILT_SOURCES) *.pcm
