#include "red.decl.h"

class start : public CBase_start {
public:
    start(CkArgMsg* m) {
        int numElements = atoi(m->argv[1]);
        ckout << "REDUCTIONS, NUM ELEMS> " << numElements << endl;
        CProxy_elem::ckNew(thisProxy, numElements);
    }

    void print(int res) {
        ckout << "Sum: " << res << endl;
        CkExit();
    }
};

class elem : public CBase_elem {
public:
    elem(CProxy_start sp) {
        int value = thisIndex;
        CkCallback cb(CkReductionTarget(start, print), sp);
        contribute(sizeof(int), &value, CkReduction::sum_int, cb);
    }
};

#include "red.def.h"