#include "allGather.decl.h"
#include "liveViz.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <random>
#include <utility>
#include <vector>

double alpha;
double beta;

// Question one ->
// Why can the bound array not directly use (inherit) the data of the
// parent array ? The two are bound to be on the same PE. Provided that they
// are of the same type.

// Question two ->
// Is it better that the allGather have only one chare array element broadcast
// the data to all the user chare array elements or is it better to have
// one-to-one message passing from bound to user chare array ?

class start : public CBase_start {
private:
  int n;
  int k;
  int x;
  int y;
  CProxy_simBox sim;
  CProxy_AllGather AllGather_array;

public:
  start(CkArgMsg *msg) {
    if (msg->argc < 3) {
      ckout << "Usage: " << msg->argv[0]
            << " <chare_array_size> <num_data_points_per_chare_array_element> "
               "<num_bits_for_pe> <num_bits_for_data_points> <alpha> <beta>"
            << endl;
      CkExit();
    }

    n = atoi(msg->argv[1]);
    k = atoi(msg->argv[2]);
    x = atoi(msg->argv[3]);
    y = atoi(msg->argv[4]);
    alpha = atoi(msg->argv[5]);
    beta = atoi(msg->argv[6]);
    delete msg;

    sim = CProxy_simBox::ckNew(thisProxy, k, n, x, y, n);
    CkArrayOptions opts(n);
    opts.bindTo(sim);
    AllGather_array = CProxy_AllGather::ckNew(k, n, sim, opts);
    sim.begin(AllGather_array);
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
  long int *data;

public:
  simBox(CProxy_start startProxy, int k, int n, int x, int y)
      : startProxy(startProxy), k(k), n(n), x(x), y(y) {
    data = (long int *)malloc(k * sizeof(long int));
    long int max_serial = (1 << y) - 1;
    long int base = CkMyPe();
    while (max_serial > 0) {
      base = base * 10;
      max_serial = max_serial / 10;
    }
    for (int i = 0; i < k; i++) {
      data[i] = base + i;
    }
  }

  void begin(CProxy_AllGather AllGather_array) {
    AllGather_array(thisIndex).begin(data, k, thisProxy);
  }

  void done(long int store[], int size) {
    free(data);
    int cont = 1;
    CkCallback cbfini(CkReductionTarget(startProxy, fini), startProxy);
    contribute(sizeof(int), &cont, CkReduction::sum_int, cbfini);
  }
};

class AllGather : public CBase_AllGather {
private:
  int k;
  int n;
  long int *store;
  int numMsg = 0;
  double timeStamp = 0.0;
  CProxy_simBox simBoxProxy;

public:
  AllGather(int k, int n, CProxy_simBox simBoxProxy)
      : k(k), n(n), simBoxProxy(simBoxProxy) {
    store = (long int *)malloc(k * n * sizeof(long int));
  }

  void begin(long int data[], int size) {
    for (int i = 0; i < k; i++) {
      store[k * thisIndex + i] = data[i];
    }
    thisProxy((thisIndex + 1) % n)
        .recv(thisIndex, data, k, (timeStamp + alpha + beta * k * 8));
    timeStamp += alpha;
  }

  void recv(int sender, long int data[], int size, double recvTime) {
    numMsg++;
    for (int i = 0; i < k; i++) {
      store[k * sender + i] = data[i];
    }
    timeStamp = max(recvTime, timeStamp);
    if (numMsg == n - 1) {
      simBoxProxy(thisIndex).done(store, k * n);
    } else {
      thisProxy((thisIndex + 1) % n)
          .recv(sender, data, k, (timeStamp + alpha + beta * k * 8));
      timeStamp += alpha;
    }
  }
};
#include "allGather.def.h"
