#include "prime.decl.h"
#include <vector>

//part A
class Main : public CBase_Main{
int totalPrime = 0;
int donePrime = 0;
std::vector<std::pair<int,int>> vec;
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
            vec.push_back(std::make_pair(p,-1));
            //pass the index to vector
            CProxy_Worker::ckNew(p,i,thisProxy);
        }
        }
    void report(int idx,bool res){
        vec[idx].second = res;
        donePrime++;
        if(donePrime==totalPrime){
            for(auto it:vec){
                ckout<<it.first<<" "<<it.second<<endl;
            }
            CkExit();
        }
    }
};

class Worker : public CBase_Worker{
public:
Worker(int p,int idx, CProxy_Main main){
    ckout<<"started worker with p "<<p<<endl;
    bool res=true;
    if(p<=1) res=false;
    for(int i=2;i<p;i++){
        if(0==p%i){
            res=false;
        }
    }
    main.report(idx,res);

}   
};

#include "prime.def.h"