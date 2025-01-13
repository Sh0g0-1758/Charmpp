#include <random>
#include "ring.decl.h"

class start : public CBase_start {
public:
    int get_rand() {
        int seed = 42;
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dis(1, 100);
        return dis(gen);
    }

    start(CkArgMsg *m) {
        int ringSize = atoi(m->argv[1]);
        int trips = atoi(m->argv[2]);
        delete m;

        CkPrintf("RingSize: %d, TripCount: %d, #PEs(): %d\n", ringSize, trips, CkNumPes());

        CProxy_ring ringArray = CProxy_ring::ckNew(thisProxy, ringSize, ringSize);
        ringArray(get_rand() % ringSize).work(ringSize, trips, -1, -1);
    }

    void done(int res) {
        ckout << "sum: " << res << endl;
        CkExit();
    }
};

class ring : public CBase_ring {
private:
    int ringSize;
    CProxy_start startProxy;
public:
    ring(CProxy_start _startProxy, int _ringSize) : startProxy(_startProxy), ringSize(_ringSize) {}
    inline int nextI() { return (thisIndex + 1) % ringSize; }
    void work(int elems_rem, int trips_rem, int from_index, int fromProc) {
        CkPrintf("Ring[%d]{%d}, TripsLeft: %d, fromRing[%d]{%d}\n", thisIndex, CkMyPe(), trips_rem, from_index, fromProc);
        int value = trips_rem;
        CkCallback cb(CkReductionTarget(start, done), startProxy);
        contribute(sizeof(int), &value, CkReduction::sum_int, cb);
        if(elems_rem > 1) {
            thisProxy(nextI()).work(elems_rem - 1, trips_rem, thisIndex, CkMyPe());
        } else if(trips_rem > 1) {
            thisProxy(nextI()).work(ringSize, trips_rem - 1, thisIndex, CkMyPe());
        }
    }
};

#include "ring.def.h"