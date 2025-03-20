#pragma once

#include "allGather.decl.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <random>
#include <utility>
#include <vector>

double alpha;
double beta;

class allGatherMsg : public CMessage_allGatherMsg {
private:
  long int *data;
public:
  void set_data(long int *d);
  long int *get_data();
  allGatherMsg(long int *d);
};

class AllGather : public CBase_AllGather {
private:
  int k = 0;
  int n = 0;
  long int *store;
  int numMsg = 0;
  double timeStamp = 0.0;
  CkCallback cb;

public:
  AllGather(int k, int n);

  void startGather(long int data[], int _, CkCallback cb);

  void recv(int sender, long int data[], int _, double recvTime);
};
