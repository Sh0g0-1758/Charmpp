#include "allGather.hh"

double alpha;
double beta;

allGatherMsg::allGatherMsg(long int *d) : data(d){};
long int *allGatherMsg::get_data() { return data; }

AllGather::AllGather(int k, int n, int type) : k(k), n(n) {
  store = (long int *)malloc(k * n * sizeof(long int));
  type = (allGatherType)type;
}

void AllGather::startGather(long int data[], int _, CkCallback cb) {
  numMsg++;
  this->cb = cb;
  for (int i = 0; i < k; i++) {
    store[k * thisIndex + i] = data[i];
  }
  thisProxy[(thisIndex + 1) % n].recv(thisIndex, data, k,
                                      (timeStamp + alpha + beta * k * 8));
  timeStamp += alpha;
  if (numMsg == n) {
    allGatherMsg *msg = new allGatherMsg(store);
    cb.send(msg);
  }
}

void AllGather::recv(int sender, long int data[], int _, double recvTime) {
  numMsg++;
  for (int i = 0; i < k; i++) {
    store[k * sender + i] = data[i];
  }
  timeStamp = std::max(recvTime, timeStamp);
  if (((thisIndex + 1) % n) != sender) {
    thisProxy[(thisIndex + 1) % n].recv(sender, data, k,
                                        (timeStamp + alpha + beta * k * 8));
    timeStamp += alpha;
  }
  if (numMsg == n) {
    allGatherMsg *msg = new allGatherMsg(store);
    cb.send(msg);
  }
}

#include "allGather.def.h"
