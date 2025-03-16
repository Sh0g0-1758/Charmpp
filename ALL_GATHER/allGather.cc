#include "liveViz.h"
#include "allGather.decl.h"
#include <cstdlib>
#include <ctime>
#include <map>
#include <random>
#include <utility>
#include <vector>
#include <cstring>

class start : public CBase_start {
private:
  int n;
  int k;
  CProxy_simBox sim;
public:
  start(CkArgMsg *msg) {
    if (msg->argc != 3) {
      ckout << "Usage: " << msg->argv[0] << " <n> <k>" << endl;
      CkExit();
    }
    n = atoi(msg->argv[1]);
    k = atoi(msg->argv[2]);
    delete msg;

    sim = CProxy_simBox::ckNew(thisProxy, k, n, n);
  }

  void fini(int numDone) {
    if (numDone == n) {
      ckout << "[STATUS] Completed the AllGather Simulation" << endl;
      CkExit();
    }
  }
};

class simBox : public CBase_simBox {
private:
  CProxy_start startProxy;
  int k;
  int n;
  std::vector<int> store;
  int seed;
  int numMsg = 0;

public:
  int gen_rand() {
    std::mt19937_64 gen(seed);
    seed++;
    std::uniform_int_distribution<int> dis(1, 10000000);
    return dis(gen);
  }

  simBox(CProxy_start startProxy, int k, int n)
      : startProxy(startProxy), k(k), n(n) {
    seed = 42 + k * thisIndex;
    store.resize(k * n);
    int data[k];
    for(int i = 0; i < k; i++) {
      data[i] = gen_rand();
      store[k * thisIndex + i] = data[i];
    }
    thisProxy((thisIndex + 1) % n).recv(thisIndex, data, k);
  }

  void recv(int sender, int data[], int size) {
    numMsg++;
    for(int i = 0; i < size; i++) {
      store[k * sender + i] = data[i];
    }
    if(numMsg == n - 1) {
      int cont = 1;
      CkCallback cbfini(CkReductionTarget(start, fini), startProxy);
      contribute(sizeof(int), &cont, CkReduction::sum_int, cbfini);
    } else {
      thisProxy((thisIndex + 1) % n).recv(sender, data, k);
    }
  }
};

#include "allGather.def.h"
