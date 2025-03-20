#include "allGather.hh"

double alpha;
double beta;

allGatherMsg::allGatherMsg(long int *d) : data(d){};
long int *allGatherMsg::get_data() { return data; }

AllGather::AllGather(int k, int n, int type) : k(k), n(n) {
  store = (long int *)malloc(k * n * sizeof(long int));
  type = (allGatherType)type;
  if(type == allGatherType::ALL_GATHER_HYPERCUBE) {
    numHypercubeIter = std::ceil((int)std::log2(n));
  }
}

void AllGather::startGather(long int data[], int _, CkCallback cb) {
  this->cb = cb;
  for (int i = 0; i < k; i++) {
    store[k * thisIndex + i] = data[i];
  }
  switch (type) {
    case allGatherType::ALL_GATHER_DEFAULT: {
      numDefaultMsg++;
      thisProxy[(thisIndex + 1) % n].recvDefault(thisIndex, data, k,
                                          (timeStamp + alpha + beta * k * 8));
      timeStamp += alpha;
      if (numDefaultMsg == n) {
        allGatherMsg *msg = new allGatherMsg(store);
        cb.send(msg);
      }
    } break;
    case allGatherType::ALL_GATHER_HYPERCUBE: {
      thisProxy(thisIndex).Hypercube();
    } break;
  }
}

void AllGather::recvDefault(int sender, long int data[], int _, double recvTime) {
  numDefaultMsg++;
  for (int i = 0; i < k; i++) {
    store[k * sender + i] = data[i];
  }
  timeStamp = std::max(recvTime, timeStamp);
  if (((thisIndex + 1) % n) != sender) {
    thisProxy[(thisIndex + 1) % n].recvDefault(sender, data, k,
                                        (timeStamp + alpha + beta * k * 8));
    timeStamp += alpha;
  }
  if (numDefaultMsg == n) {
    allGatherMsg *msg = new allGatherMsg(store);
    cb.send(msg);
  }
}

#include "allGather.def.h"
