#include "fib.decl.h"
#define Threshold 10
class Main:public CBase_Main{
public:
    Main(CkArgMsg* m){
        int n = atoi(m->argv[1]);
        CProxy_Fib::ckNew(true,n,CProxy_Fib());
    }
};
class Fib:public CBase_Fib{
CProxy_Fib parent;
bool isRoot;
int counter;
int res;
int n_;
public:
    Fib(bool isRoot_,int n,CProxy_Fib parent_):parent(parent_), isRoot(isRoot_),counter(0),n_(n),res(0) {
        ckout<<"created Fib chare with n = "<<n<<endl;
        if(n<=Threshold){
            int ans1 = fib_seq(n-1);
            int ans2 = fib_seq(n-2);
            thisProxy.result(ans1);
            thisProxy.result(ans2);
        }
        else{
            CProxy_Fib::ckNew(false, n-1,thisProxy);
            CProxy_Fib::ckNew(false,n-2,thisProxy);
        }
    }
    void result(int val){
        counter++;
        res+=val;
        if(counter==2){
            if(isRoot){
                ckout<<"result for n = "<<n_<<" is "<<res<<endl;
                CkExit();
            }
            parent.result(res);
        }
    }
    int fib_seq(int n){
        return n<2?n:fib_seq(n-1)+fib_seq(n-2);
    }
};


#include "fib.def.h"