#include<stdlib.h>

#include "dart.decl.h"

class start : public CBase_start {
private:
    int circle;
    int tot;
    int total_work;
    int curr;
    double start_time;
public:
    start(CkArgMsg* m) : circle(0), tot(20), curr(0) {
        if(m -> argc != 2) {
            ckout << "Usage: " << m -> argv[0] << " <number of darts>" << endl;
        }
        total_work = atoi(m -> argv[1]);
        int grainsize = total_work / tot;
        CProxy_worker workers[tot];
        start_time = CkWallTimer();
        for(int i = 0; i < tot; i ++) {
            workers[i] = CProxy_worker::ckNew(grainsize, i + 1, thisProxy);
            workers[i].throw_dart();
        }
    }

    void record(int c) {
        circle += c;
        curr++;
        if(curr == tot) {
            double end_time = CkWallTimer();
            ckout << "Time: " << end_time - start_time << " seconds" << endl;
            ckout << "Pi is approximately " << 4.0 * circle / total_work << endl;
            CkExit();
        }
    }
};


class worker : public CBase_worker {
private:
    int work;
    int rand_seed;
    CProxy_start start_proxy;
public:
    worker(int w, int r, CProxy_start s) : work(w), rand_seed(r), start_proxy(s) {
        srand(rand_seed);
    }

    void throw_dart() {
        int circle = 0;
        for(int i = 0; i < work; i++) {
            double x = drand48();
            double y = drand48();
            if(x * x + y * y <= 1) circle++;
        }
        start_proxy.record(circle);
    }
};

#include "dart.def.h"
