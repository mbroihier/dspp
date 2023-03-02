#ifndef WSPRSYMBOLS_H_
#define WSPRSYMBOLS_H_
/*
 *      WSPRSymbols.h - Find WSPR symbols in an FFT stream
 *
 *      Copyright (C) 2022 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <list>
#include <map>
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class WSPRSymbols {
 private:
  void init(int size, int number, char * prefix);
  void adjustTargets(float centroid, int candidate);
  int findClosestTarget (float centroid, int candidate);
  void logCentroid(float centroid, int candidate);
  void logBase(float baseValue, int candidate);
  void reportHistory(int numberOfCandidates);
  void convertToSymbols(std::list<float> centroidList);
  void regressionFit(std::list<float> centroidList);
  int * binArray;
  bool * used;
  float * samples;
  float * mag;
  float slope;
  float yIntercept;
  int * histogram;
  int sampleBufferSize;
  int size;
  int number;
  int tic;
  char * prefix;

  const int FIRST = 0;
  const int SECOND = 1;
  const int THIRD = 2;
  const int REF1 = 4;
  const int REF2 = 5;

  const float BW = 3.0;
  const float BW_DELTA = BW / 3.0;
  const float BW1 = BW / 6.0;
  const float BW2 = BW1 + BW_DELTA;
  const float BW3 = BW2 + BW_DELTA;

  const float CF = BW / 2.0;
  const float LOWER = CF - BW1;
  const float HIGHER = CF + BW1;

  const int TARGET0 = 0;
  const int TARGET1 = 1;
  const int TARGET2 = 2;
  const int TARGET3 = 3;
  
  std::map<int, std::map<int, float> *>  targets;

  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  struct Range { float lowerBound; float upperBound; };
  struct BaseRecord { float base; int timeStamp; };
  std::map<int, float> candidates;    // list of cadidates mapped to their centroid location
  std::map<int, bool> floatingCandidateUpdated; 
  
  std::map<int, std::list<SampleRecord> *> centroidHistory;
  std::map<int, std::list<BaseRecord> *> baseHistory;
  struct CandidateRecord { float centroid; int timeStamp; Range range; std::list<SampleRecord> * history;
    int floatingCandidateID; bool groupIndexUsed;};
  CandidateRecord * allCandidates;
  bool * alreadyUpdated;
 public:
  void doWork();
  WSPRSymbols(int size, int number, char * prefix);
  ~WSPRSymbols(void);
};
#endif  // WSPRSYMBOLS_H_