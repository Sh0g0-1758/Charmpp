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
  int numMsg = 0;
  double timeStamp = 0.0;
  CkCallback cb;
  allGatherType type;

public:
  AllGather(int k, int n, int type);

  void startGather(long int data[], int _, CkCallback cb);

  void recv(int sender, long int data[], int _, double recvTime);
};
