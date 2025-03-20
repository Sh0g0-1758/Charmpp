#include "allGather.hh"

double alpha;
double beta;

allGatherMsg::allGatherMsg(long int *d) : data(d){};
long int *allGatherMsg::get_data() { return data; }

int AllGather::gen_rand() {
  std::mt19937_64 gen(randCounter++);
  std::uniform_int_distribution<int> dis(0, n - 1);
  return dis(gen);
}

AllGather::AllGather(int k, int n, int type) : k(k), n(n) {
  store = (long int *)malloc(k * n * sizeof(long int));
  type = (allGatherType)type;
  switch (type) {
  case allGatherType::ALL_GATHER_HYPERCUBE: {
    numHypercubeIter = std::ceil((int)std::log2(n));
  } break;
  case allGatherType::ALL_GATHER_FLOODING: {
    graph.resize(n);
    for (int i = 0; i < n; i++) {
      graph[i].resize(n);
    }
    // Create a connected graph
    // Ring
    for (int i = 0; i < n; i++) {
      graph[i][(i + 1) % n] = 1;
      graph[i][(i - 1) % n] = 1;
    }
    // Random [n/2] connections
    for (int i = 0; i < (int)(n / 2); i++) {
      int x = gen_rand();
      int y = gen_rand();
      if (x != y) {
        graph[x][y] = 1;
        graph[y][x] = 1;
      }
    }
  } break;
  case allGatherType::ALL_GATHER_DEFAULT:
    break;
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
    thisProxy[(thisIndex + 1) % n].recvDefault(
        thisIndex, data, k, (timeStamp + alpha + beta * k * 8));
    timeStamp += alpha;
    if (numDefaultMsg == n) {
      allGatherMsg *msg = new allGatherMsg(store);
      cb.send(msg);
    }
  } break;
  case allGatherType::ALL_GATHER_HYPERCUBE: {
    thisProxy(thisIndex).Hypercube();
  } break;
  case allGatherType::ALL_GATHER_FLOODING: {
    numAccFloodMsg++;
    recvFloodMsg[thisIndex] = true;
    for (int i = 0; i < n; i++) {
      if (graph[thisIndex][i] == 1) {
        thisProxy(i).Flood(thisIndex, data, k,
                           (timeStamp + alpha + beta * k * 8));
        timeStamp += alpha;
      }
    }
    if (numAccFloodMsg == n) {
      allGatherMsg *msg = new allGatherMsg(store);
      cb.send(msg);
    }
  } break;
  }
}

void AllGather::recvDefault(int sender, long int data[], int _,
                            double recvTime) {
  numDefaultMsg++;
  for (int i = 0; i < k; i++) {
    store[k * sender + i] = data[i];
  }
  timeStamp = std::max(recvTime, timeStamp);
  if (((thisIndex + 1) % n) != sender) {
    thisProxy[(thisIndex + 1) % n].recvDefault(
        sender, data, k, (timeStamp + alpha + beta * k * 8));
    timeStamp += alpha;
  }
  if (numDefaultMsg == n) {
    allGatherMsg *msg = new allGatherMsg(store);
    cb.send(msg);
  }
}

void AllGather::Flood(int sender, long int data[], int _, double recvTime) {
  if (recvFloodMsg[sender]) {
    return;
  }
  numAccFloodMsg++;
  recvFloodMsg[sender] = true;
  for (int i = 0; i < k; i++) {
    store[k * sender + i] = data[i];
  }
  timeStamp = std::max(recvTime, timeStamp);
  for (int i = 0; i < n; i++) {
    if (graph[thisIndex][i] == 1 and i != sender) {
      thisProxy(i).Flood(sender, data, k, (timeStamp + alpha + beta * k * 8));
      timeStamp += alpha;
    }
  }
  if (numAccFloodMsg == n) {
    allGatherMsg *msg = new allGatherMsg(store);
    cb.send(msg);
  }
}

#include "allGather.def.h"
