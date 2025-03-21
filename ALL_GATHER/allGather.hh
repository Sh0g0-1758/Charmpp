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
  int k{};
  int n{};
  long int *store;
  int numDefaultMsg{};
  double timeStamp{};
  CkCallback cb;
  allGatherType type;
  int numHypercubeIter{};
  int iter;
  int HypercubeToSend;
  int numAccFloodMsg{};
  std::vector<std::vector<int>> graph{};
  std::map<int, bool> recvFloodMsg{};
  int randCounter{};
  std::vector<int> hyperCubeIndx{};
  std::vector<long int> hyperCubeStore{};

public:
  AllGather_SDAG_CODE

  AllGather(int k, int n, int type);

  void startGather(long int data[], int _, CkCallback cb);

  void recvDefault(int sender, long int data[], int _, double recvTime);

  int gen_rand();

  void Flood(int sender, long int data[], int _, double recvTime);
};
