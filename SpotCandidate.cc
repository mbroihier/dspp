/*
 *      SpotCandidate.cc - create an object that contains information regarding a potential spot
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include "SpotCandidate.h"

/* ---------------------------------------------------------------------- */
SpotCandidate::SpotCandidate(int ID, float deltaFreq) {
  this->ID = ID;
  this->deltaFreq = deltaFreq;
  count = 0;
  longestSequence = 0;
  lastTimeStamp = -2;
  currentSequence = 0;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
}
/* ---------------------------------------------------------------------- */
SpotCandidate::SpotCandidate(int ID, const std::vector<SampleRecord> input, float deltaFreq) {
  this->ID = ID;
  this->deltaFreq = deltaFreq;
  count = 0;
  longestSequence = 0;
  lastTimeStamp = -2;
  currentSequence = 0;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
  StartEnd se = { 0, 0 };
  for (auto sr : input) {
    candidateVector.push_back(sr);
    if (lastTimeStamp < 0) {
      se.start = sr.timeStamp;
    } else {
      se.end = sr.timeStamp;
    }
    currentSequence++;
    lastTimeStamp = sr.timeStamp;
  }
  sequenceDelimiters.push_back(se);
  longestSequence = currentSequence;
  count = currentSequence;
  if (longestSequence > 161) {
    valid = true;
    fitInfo = new Regression(getCentroidVector());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
    if (ID > 127) {
      freq = (yIntercept + (ID - 256)) * deltaFreq;  // NEED TO MAKE DYNAMIC
    } else {
      freq = (yIntercept + ID) * deltaFreq;
    }
    minCentroid = fitInfo->getMinCentroid();
    maxCentroid = fitInfo->getMaxCentroid();
  }
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::logSample(float centroid, float magnitude, int timeStamp, float timeSeconds) {
  if (timeStamp == lastTimeStamp) {
    return false;  // return that a candidate was already logged
  } else {
    if ((lastTimeStamp + 1) == timeStamp) {
      currentSequence++;
      std::vector<StartEnd>::iterator current = sequenceDelimiters.end();
      current--;
      StartEnd se = *current;
      se.end = timeStamp;
      *current = se;
    } else {
      StartEnd se;
      se.start = timeStamp;
      se.end = timeStamp;
      sequenceDelimiters.push_back(se);
      currentSequence = 1;
    }
    if (currentSequence > longestSequence) {
      longestSequence = currentSequence;
    }
    SampleRecord sr;
    sr.centroid = centroid;
    sr.magnitude = magnitude;
    sr.timeStamp = timeStamp;
    sr.timeSeconds = timeSeconds;
    candidateVector.push_back(sr);
    count++;
    lastTimeStamp = timeStamp;
    if (longestSequence > 161) {
      if (fitInfo) delete(fitInfo);
      fitInfo = new Regression(getCentroidVector());
      slope = fitInfo->getSlope();
      yIntercept = fitInfo->getYIntercept();
      valid = true;
    }
  }
  return true;
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::mergeVector(const std::vector<SampleRecord> other) {
  std::vector<SampleRecord> newVector;
  if (other.size() == 0) return false;
  if (candidateVector.size() == 0) return false;
  int workingTime = 0;
  int lastWorkingTime = -2;
  int longestSequence = 0;
  int currentSequence = 1;
  int count = 0;
  bool done = false;
  int merged = 0;
  std::vector<SampleRecord>::iterator iter1 = candidateVector.begin();
  std::vector<SampleRecord>::const_iterator iter2 = other.begin();
  sequenceDelimiters.clear();
  if (iter1 == candidateVector.end()) {
    fprintf(stderr, "Can't merge into empty vector\n");
    return false;
  }
  if (iter2 == other.end()) {
    fprintf(stderr, "Merging an empty vector does nothing\n");
    return false;
  }
  SampleRecord one = *iter1;
  SampleRecord two = *iter2;
  bool finishUsingTargetVector = false;
  bool finishUsingOtherVector = false;
  while (! done) {
    if (iter1 != candidateVector.end()) {  // don't go past last entry
      one = *iter1;
    } else {
      finishUsingOtherVector = true;
      fprintf(stderr, "The end of the target list has been reached\n");
    }
    if (iter2 != other.end()) {  // don't go past last entry
      two = *iter2;
    } else {
      fprintf(stderr, "The end of the list being merged has been reached\n");
      finishUsingTargetVector = true;
    }
    if (finishUsingTargetVector || finishUsingOtherVector) {
      if (finishUsingTargetVector) {
        workingTime = one.timeStamp;
        newVector.push_back(one);
        fprintf(stderr, "recorded an element from the target list and advancing\n");
        count++;
        iter1++;
      } else {
        workingTime = two.timeStamp;
        newVector.push_back(two);
        fprintf(stderr, "recorded an element from the list to merge in and advancing\n");
        merged++;
        count++;
        iter2++;
      }
    } else {
      workingTime = one.timeStamp;
      if (workingTime > two.timeStamp) {
        workingTime = two.timeStamp;
      }
      if (workingTime == one.timeStamp) {
        newVector.push_back(one);
        fprintf(stderr, "recorded an element from the target list and advancing\n");
        count++;
        if (iter1 != candidateVector.end()) iter1++;
        if (workingTime >= two.timeStamp) {  // we skip this record of the other vector
          while (iter2 != other.end() && workingTime >= (*iter2).timeStamp) {
            fprintf(stderr, "Incrementing other pointer\n");
            iter2++;
          }
        }
      } else {
        fprintf(stderr, "Stitching at working time %d, two.timeStamp %d\n", workingTime, two.timeStamp);
        if (workingTime == two.timeStamp) {
          newVector.push_back(two);
          fprintf(stderr, "recorded an element from the list to merge in and advancing\n");
          merged++;
          count++;
          if (iter2 != other.end()) iter2++;
          if (workingTime >= one.timeStamp) {  // we skip this record of candidate vector
            while (iter1 != candidateVector.end() && workingTime >= (*iter1).timeStamp) {
              fprintf(stderr, "Incrementing target pointer\n");
              iter1++;
            }
          }
        }
      }
    }
    if ((sequenceDelimiters.size() > 0) && (workingTime == lastWorkingTime + 1)) {
      std::vector<StartEnd>::iterator current = sequenceDelimiters.end();
      current--;
      StartEnd se = *current;
      se.end = workingTime;
      *current = se;
      currentSequence++;
      fprintf(stderr, "currentSequence %d, start %d, end %d, diff %d\n", currentSequence, se.start, se.end,
              se.end - se.start + 1);
    } else {
      currentSequence = 1;
      StartEnd se = {workingTime, workingTime};
      sequenceDelimiters.push_back(se);
    }          
    if (currentSequence > longestSequence) {
      longestSequence = currentSequence;
    }
    lastWorkingTime = workingTime;
    done = (iter1 == candidateVector.end()) && (iter2 == other.end());
  }
  this->longestSequence = longestSequence;
  this->count = count;
  candidateVector.clear();
  for (auto entry : newVector) {
    candidateVector.push_back(entry);
  }
  if (longestSequence > 161) {
    if (fitInfo) delete(fitInfo);
    fitInfo = new Regression(getCentroidVector());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
    valid = true;
  } else {
    valid = false;
  }
  fprintf(stderr, "Merged %d samples from other list\n", merged);
  return true;
}
/* ---------------------------------------------------------------------- */
const std::vector<SpotCandidate::SampleRecord> SpotCandidate::getVector(void) {
  return candidateVector;
}
/* ---------------------------------------------------------------------- */
const std::vector<SpotCandidate::SampleRecord> SpotCandidate::getValidSubvector(int vectorNumber) {
  StartEnd se = { 0, 0 };
  int validCount = 0;
  aSubvector.clear();
  for (std::vector<StartEnd>::iterator iter = sequenceDelimiters.begin(); iter != sequenceDelimiters.end(); iter++) {
    se = *iter;
    if (se.end - se.start + 1 > 161) { // valid
      validCount++;
      if (validCount == vectorNumber) {  // get this list
        break;
      }
    }
    se = { 0, 0 };
  }
  if (se.start == se.end) return aSubvector;  // return an empty list if no list was found
  for (auto entry : candidateVector) {
    if (entry.timeStamp >= se.start && entry.timeStamp <= se.end) {
      aSubvector.push_back(entry);
    }
  }
  return aSubvector;
}
/* ---------------------------------------------------------------------- */
void SpotCandidate::tokenize(const std::vector<SampleRecord> validVector, std::vector<int> & tokens) {
  int interleavedSync [] = { 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0,
                             0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1,
                             0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
                             1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                             0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0,
                             0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1,
                             0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1,
                             0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0,
                             0, 0 };

  tokens.clear();
  SpotCandidate candidate(1000, validVector, 0.0);
  std::vector<float> magnitudeAverages;
  for (int i = 0; i < WINDOW; i++) {
    float sum = 0.0;
    for (auto entry : validVector) {
      sum += entry.magSlice[i];
    }
    magnitudeAverages.push_back(sum / validVector.size());
  }
  std::vector<float> centroidVector = candidate.getCentroidVector();
  float slope = candidate.getSlope();
  float minCentroid = candidate.getMinCentroid();
  float maxCentroid = candidate.getMaxCentroid();
  float x = 0.0;
  float scale = 255.0/(maxCentroid - minCentroid);
  fprintf(stderr, "bounds observed were: %7.2f and %7.2f\n", minCentroid, maxCentroid);
  float base = 0.0;
  if (slope < 0.0) {  // The maximum value of the signal should occur near zero, the minimum value near count
    base = minCentroid - slope * (validVector.size() - 1);
  } else {
    base = minCentroid;
  }
  base = candidate.getYIntercept() - 1.5;  // EXPERIMENNT
  // the vector below should result in a call sign of KG5YJE, a location of EM13 and power of 10
  int testInput[] = {3, 3, 2, 2, 2, 2, 2, 2, 3, 0, 2, 0, 3, 1, 1, 0, 0, 0, 1, 2, 2, 1, 2, 1, 1, 1, 3, 0, 0, 2,
                     2, 2, 2, 2, 1, 0, 2, 3, 0, 1, 0, 0, 0, 0, 0, 2, 1, 0, 3, 3, 2, 0, 1, 3, 2, 3, 2, 2, 2, 3,
                     3, 0, 3, 0, 0, 2, 2, 3, 1, 0, 1, 2, 3, 0, 3, 2, 3, 2, 0, 1, 0, 0, 1, 2, 1, 1, 2, 0, 0, 3,
                     3, 0, 1, 2, 1, 2, 2, 2, 1, 0, 0, 0, 0, 2, 3, 0, 0, 3, 0, 0, 1, 3, 1, 2, 3, 3, 0, 2, 1, 3,
                     0, 1, 0, 0, 2, 3, 1, 1, 2, 2, 2, 0, 0, 3, 2, 1, 0, 0, 1, 3, 2, 0, 2, 2, 0, 0, 0, 1, 1, 2,
                     3, 0, 3, 1, 2, 0, 0, 3, 3, 2, 0, 2};
#ifdef SELFTEST
  base = 0.0;
  //scale = 60.0; taking this out gives some randomness in the output
  //slope = 0.0;  taking this out gives some randomness in the output
  centroidVector.clear();
  int index = 0;
  for (auto entry : testInput) {
    centroidVector.push_back(entry);
    if (((entry & 0x01) ^ interleavedSync[index++]) == 1) {
      fprintf(stderr, "Sync error at location %d\n", index - 1);
      return;
    }
  }
#endif
  int syncIndex = 0;
  int token = 0;
  int cbToken = 0;  // centroid based token
  int sumAE0 = 0;
  int sumAE1 = 0;
  for (auto entry : centroidVector) {
    int sliceIndexZero = (int) (base - 0.5);
    int sliceIndexOne = sliceIndexZero + 1;
    int sliceIndexTwo = sliceIndexZero + 2;
    int sliceIndexThree = sliceIndexZero + 3;
    if (sliceIndexZero < 0 || sliceIndexThree >= WINDOW) {
      fprintf(stderr, "Can not tokenize this vector\n");
      tokens.clear(); // clear anything that may have been entered into the vector
      break;
    }
    fprintf(stderr, " %15.0f, %15.0f, %15.0f, %15.0f, %d,",
            validVector[syncIndex].magSlice[sliceIndexZero] - magnitudeAverages[sliceIndexZero],
            validVector[syncIndex].magSlice[sliceIndexOne] - magnitudeAverages[sliceIndexOne],
            validVector[syncIndex].magSlice[sliceIndexTwo] - magnitudeAverages[sliceIndexTwo],
            validVector[syncIndex].magSlice[sliceIndexThree] - magnitudeAverages[sliceIndexThree],
            interleavedSync[syncIndex]);
    if (interleavedSync[syncIndex] == 1) {
      // token should be 64 or 192
      if (validVector[syncIndex].magSlice[sliceIndexOne] - magnitudeAverages[sliceIndexOne] <
          validVector[syncIndex].magSlice[sliceIndexThree] - magnitudeAverages[sliceIndexThree]) {
        token = 3 << 6;
      } else {
        token = 1 << 6;
      }
    } else { // token should be 0 or 128
      if (validVector[syncIndex].magSlice[sliceIndexZero] - magnitudeAverages[sliceIndexZero] <
          validVector[syncIndex].magSlice[sliceIndexTwo] - magnitudeAverages[sliceIndexTwo]) {
        token = 2 << 6;
      } else {
        token = 0;
      }
    }
    fprintf(stderr, " token: %3d, ideal: %3d\n", token, testInput[syncIndex] << 6);
    cbToken = std::max(std::min((int)((entry - base) * scale + 0.5),255), 0);
    tokens.push_back(token);
    //tokens.push_back(cbToken);  //EXPERIMENT
    sumAE0 += abs(token - (testInput[syncIndex] << 6));
    sumAE1 += abs(cbToken - (testInput[syncIndex] << 6));
    //fprintf(stderr, "sample %3d - minCentroid: %7.2f, base: %7.2f, actual: %7.2f, error: %7.2f, token: %3d, "
    //        "cbToken: %3d, ideal: %3d\n",
    //        (int) x, minCentroid, base, entry, entry - base, token, cbToken, testInput[syncIndex] << 6);
    x += 1.0;
    base += slope;
    syncIndex++;
  }
  fprintf(stderr, "sumAE0: %8d, sumAE1: %8d\n", sumAE0, sumAE1);
}
/* ---------------------------------------------------------------------- */
std::vector<float> SpotCandidate::getCentroidVector(void) {
  centroids.clear();
  for (auto entry : candidateVector) {
    centroids.push_back(entry.centroid);
  }
  return centroids;
}
/* ---------------------------------------------------------------------- */
std::vector<float> SpotCandidate::getMagnitudeVector(void) {
  magnitudes.clear();
  for (auto entry : candidateVector) {
    magnitudes.push_back(entry.magnitude);
  }
  return magnitudes;
}
/* ---------------------------------------------------------------------- */
void SpotCandidate::printReport(void) {
  fprintf(stderr, "Potential Candidate %d Report - samples: %5d, longest sequence: %5d, status: %s, slope: %7.4f, y-intercept: %7.2f, center frequency of spot: %8.5f\n",
          ID, count, longestSequence, valid?"  valid":"invalid", slope, yIntercept, freq);
  int i = 0;
  if (candidateVector.size() < 1) {
    fprintf(stderr, "No information on candidate\n");
    return;
  }
  lastTimeStamp = -1;
  for (auto entry : candidateVector) {
    fprintf(stderr, "%3d: centroid: %7.2f, magnitude: %10.0f, time stamp: %5d, time in seconds: %7.2f %s\n",
            i++, entry.centroid, entry.magnitude, entry.timeStamp, entry.timeSeconds,
            ((entry.timeStamp - lastTimeStamp) == 1)?"*":" ");
    lastTimeStamp = entry.timeStamp;
  }
  for (auto entry : sequenceDelimiters) {
    if (entry.start != entry.end) fprintf(stderr, "sequence start %d, sequence end %d\n", entry.start, entry.end);
  }
  fprintf(stderr, "Magnitude slice\n");
  int line = 0;
  float acc = 0.0;
  for (auto entry : candidateVector) {
    for (int i = 0; i < WINDOW; i++) {
      acc += entry.magSlice[i];
      fprintf(stderr, "%9.0f,", entry.magSlice[i]);
    }
    fprintf(stderr, " %d\n", line++);
  }
  line = 0;
  float average = acc / (candidateVector.size() * WINDOW);
  fprintf(stderr, "Magnitude graphic\n");
  for (auto entry : candidateVector) {
    if (entry.magSlice.size() == 0) break;
    char graphic[WINDOW + 1];
    if (entry.magSlice[0] > entry.magSlice[1] && entry.magSlice[0] > average) {
      graphic[0] = '*';
    } else {
      graphic[0] = '_';
    }
    for (int i = 1; i < WINDOW - 1 ; i++) {
      if (entry.magSlice[i - 1] < entry.magSlice[i] && entry.magSlice[i] > entry.magSlice[i + 1]
          && entry.magSlice[i] > average) {
        graphic[i] = '*';
      } else {
        graphic[i] = '_';
      }
    }
    if (entry.magSlice[WINDOW - 1] > entry.magSlice[WINDOW - 2] && entry.magSlice[WINDOW - 1] > average) {
      graphic[WINDOW - 1] = '*';
    } else {
      graphic[WINDOW - 1] ='_';
    }
    graphic[WINDOW] = '\0';
    fprintf(stderr, " %s %d\n", graphic, line++);
  }
}
/* ---------------------------------------------------------------------- */
SpotCandidate::~SpotCandidate(void) {
  candidateVector.clear();
  if (fitInfo) delete(fitInfo);
}
