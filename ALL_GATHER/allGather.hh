#pragma once

#include "allGather.decl.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
#include <random>
#include <utility>
#include <vector>

class allGatherMsg : public CMessage_allGatherMsg {
private:
  long int *data;

public:
  long int *get_data();
  allGatherMsg(long int *d);
};

enum allGatherType {
  ALL_GATHER_DEFAULT,
  ALL_GATHER_HYPERCUBE,
  ALL_GATHER_FLOODING
};

class AllGather : public CBase_AllGather {
private:
  // COMMON
  int k = 0;
  int n = 0;
  long int *store;
  allGatherType type;
  CkCallback cb;
  double timeStamp = 0.0;
  // ALL_GATHER_DEFAULT
  int numDefaultMsg = 0;
  // ALL_GATHER_HYPERCUBE
  int numHypercubeIter = 0;
  int iter;
  int HypercubeToSend;
  // ALL_GATHER_FLOODING
  int numAccFloodMsg = 0;
  std::vector<std::vector<int>> graph;
  std::map<int, bool> recvFloodMsg;
  int randCounter = 0;

public:
  AllGather_SDAG_CODE

  AllGather(int k, int n, int type);

  void startGather(long int data[], int _, CkCallback cb);

  void recvDefault(int sender, long int data[], int _, double recvTime);

  void Flood(int sender, long int data[], int _, double recvTime);

  int gen_rand();
};
