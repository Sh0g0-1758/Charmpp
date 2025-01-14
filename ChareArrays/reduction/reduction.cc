#include "Reduction.decl.h"

class Main: public CBase_Main{
public:
    Main(CkArgMsg* m){
        // CProxy_Ele e = CProxy_Ele::ckNew(thisProxy,0);//blocks
        CProxy_Ele e = CProxy_Ele::ckNew(thisProxy);//also blocks
        CkPrintf("Starting reduction\n");
    }
    void result(int result){
        CkPrintf("Result: %d\n",result);
        CkExit();
    }
};

//reduction does not complete until all elements have contributed
class Ele: public CBase_Ele{
public:
    Ele(CProxy_Main m){
        CkCallback cb(CkReductionTarget(Main,result),m);
        contribute(sizeof(int),&thisIndex,CkReduction::sum_int,cb);
    }

};

#include "Reduction.def.h"