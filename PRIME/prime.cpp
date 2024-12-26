#include "prime.decl.h"
#include <map>

//part A
class Main : public CBase_Main{
int totalPrime = 0;
int donePrime = 0;
std::map<int,int> mp;
public:
    Main(CkArgMsg* m){
        if(m->argc!=2){
            ckout<<"usage: ./prime <num>"<<endl;
        }  
        srand(time(NULL));
        int K = atoi(m->argv[1]);  
        totalPrime = K;
        for(int i=0;i<K;i++){
            int p = rand();
            if(mp.count(p)!=0){continue;}
            mp[p]=-1;
            CProxy_Worker::ckNew(p,thisProxy);
        }
        }
    void report(int p,bool res){
        ckout<<"reported "<<p<<" is "<<res<<endl;
        mp[p] = res; 
        donePrime++;
        if(donePrime==totalPrime){
            for(auto r:mp){
                ckout<<r.first<<" "<<r.second<<endl;
            }
            CkExit();
        }
    }
};

class Worker : public CBase_Worker{
public:
Worker(int p, CProxy_Main main){
    ckout<<"started worker with p "<<p<<endl;
    bool res=true;
    if(p<=1) res=false;
    for(int i=2;i<p;i++){
        if(0==p%i){
            res=false;
        }
    }
    main.report(p,res);

}   
};

#include "prime.def.h"