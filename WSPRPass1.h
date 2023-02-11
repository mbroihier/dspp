#ifndef WSPRPASS1_H_
#define WSPRPASS1_H_
/*
 *      WSPRPass1.h - Find WSPR potential WSPR signal
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <list>
#include <map>
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class WSPRPass1 {
 private:
  const int NOMINAL_NUMBER_OF_SYMBOLS = 162;
  void init(int size, int number, char * prefix);
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
  float freq;
  char * prefix;

  std::map<int, std::map<int, float> *>  targets;

  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  struct Range { float lowerBound; float upperBound; };
  struct BaseRecord { float base; int timeStamp; };
  std::map<int, float> candidates;    // list of cadidates mapped to their centroid location
  
  std::map<int, std::list<SampleRecord> *> centroidHistory;
  std::map<int, std::list<BaseRecord> *> baseHistory;
  struct CandidateRecord { float centroid; int timeStamp; Range range; std::list<SampleRecord> * history;
    bool groupIndexUsed;};
  CandidateRecord * allCandidates;
  bool * alreadyUpdated;
 public:
  void doWork(void);
  WSPRPass1(int size, int number, char * prefix);
  ~WSPRPass1(void);
};
#endif  // WSPRPASS1_H_
