#pragma once

#include "allGather.decl.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>
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
  int k = 0;
  int n = 0;
  long int *store;
  int numDefaultMsg = 0;
  double timeStamp = 0.0;
  CkCallback cb;
  allGatherType type;
  int numHypercubeIter = 0;
  int iter;
  int HypercubeToSend;

public:
  AllGather_SDAG_CODE

  AllGather(int k, int n, int type);

  void startGather(long int data[], int _, CkCallback cb);

  void recvDefault(int sender, long int data[], int _, double recvTime);
};
