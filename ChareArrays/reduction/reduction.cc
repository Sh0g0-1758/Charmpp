#include "Reduction.decl.h"

class Main: public CBase_Main{
public:
    Main(CkArgMsg* m){
        // CProxy_Ele e = CProxy_Ele::ckNew(thisProxy,0);//blocks
        CProxy_Ele e = CProxy_Ele::ckNew(thisProxy,10);
    }
    void result(int result){
        CkPrintf("Result: %d\n",result);
        CkExit();
    }
};

//reduction does not complete until all elements have contributed
class a: public CBase_Ele{
public:
    a(CProxy_Main m){
        ckout<<"Ele["<<thisIndex<<"] created on ("<<CkMyPe()<<")"<<endl;
    }

};

class b: public CBase_Ele{
public:
    b(CProxy_Main m){
        ckout<<"EleTest["<<thisIndex<<"] created on ("<<CkMyPe()<<")"<<endl;
    }
};
#include "Reduction.def.h"