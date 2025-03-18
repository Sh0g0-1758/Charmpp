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
  int x;
  int y;
  CProxy_simBox sim;
public:
  start(CkArgMsg *msg) {
    if (msg->argc < 3) {
      ckout << "Usage: " << msg->argv[0] << " <chare_array_size> <num_data_points_per_chare_array_element> <num_bits_for_pe | default = 20> <num_bits_for_data_points | default = 30>" << endl;
      CkExit();
    }
    n = atoi(msg->argv[1]);
    k = atoi(msg->argv[2]);
    if(msg->argc == 3) {
      x = 20;
      y = 30;
    } else if(msg->argc == 4) {
      x = atoi(msg->argv[3]);
      y = 30;
    } else {
      x = atoi(msg->argv[3]);
      y = atoi(msg->argv[4]);
    }
    delete msg;

    sim = CProxy_simBox::ckNew(thisProxy, k, n, x, y, n);
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
  int x;
  int y;
  long int* store;
  int numMsg = 0;

public:
  simBox(CProxy_start startProxy, int k, int n, int x, int y)
      : startProxy(startProxy), k(k), n(n), x(x), y(y) {
    store = (long int*)malloc(k * n * sizeof(long int));
    long int data[k];
    long int max_serial = (1 << y) - 1;
    long int base = CkMyPe();
    while(max_serial > 0) {
      base = base * 10;
      max_serial = max_serial / 10;
    }
    for(int i = 0; i < k; i++) {
      data[i] = base + i;
      store[k * thisIndex + i] = data[i];
    }
    thisProxy((thisIndex + 1) % n).recv(thisIndex, data, k);
  }

  void recv(int sender, long int data[], int size) {
    numMsg++;
    for(int i = 0; i < size; i++) {
      store[k * sender + i] = data[i];
    }
    if(numMsg == n - 1) {
      free(store);
      int cont = 1;
      CkCallback cbfini(CkReductionTarget(start, fini), startProxy);
      contribute(sizeof(int), &cont, CkReduction::sum_int, cbfini);
    } else {
      thisProxy((thisIndex + 1) % n).recv(sender, data, k);
    }
  }
};

#include "allGather.def.h"
