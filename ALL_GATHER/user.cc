#include "user.hh"

start::start(CkArgMsg *msg) {
  if (msg->argc < 3) {
    ckout << "Usage: " << msg->argv[0]
          << " <chare_array_size> <num_data_points_per_chare_array_element> "
             "<num_bits_for_pe> <num_bits_for_data_points>"
          << endl;
    CkExit();
  }

  n = atoi(msg->argv[1]);
  k = atoi(msg->argv[2]);
  x = atoi(msg->argv[3]);
  y = atoi(msg->argv[4]);
  delete msg;

  sim = CProxy_simBox::ckNew(thisProxy, k, n, x, y, n);

  CkArrayOptions opts(n);
  opts.bindTo(sim);
  AllGather_array = CProxy_AllGather::ckNew(k, n, opts);

  sim.begin(AllGather_array);
}

void start::fini(int numDone) {
  if (numDone == n) {
    ckout << "[STATUS] Completed the AllGather Simulation" << endl;
    CkExit();
  }
}

simBox::simBox(CProxy_start startProxy, int k, int n, int x, int y)
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

void simBox::begin(CProxy_AllGather AllGather_array) {
  CkCallback cb(CkIndex_simBox::done(NULL), CkArrayIndex1D(thisIndex),
                thisProxy);
  AllGather_array(thisIndex).startGather(data, k, cb);
}

void simBox::done(allGatherMsg *msg) {
  result = msg->get_data();
  int cnt = 1;
  ckout << "Data from chare " << thisIndex << " : " << endl;
  for (int i = 0; i < k * n; i++) {
    ckout << result[i] << " ";
  }
  ckout << endl;
  CkCallback cbfini(CkReductionTarget(start, fini), startProxy);
  contribute(sizeof(int), &cnt, CkReduction::sum_int, cbfini);
}
