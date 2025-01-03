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
        srand(42);
        int K = atoi(m->argv[1]);  
        vec.resize(K);
        totalPrime = K;
        for(int i=0;i<K;i++){
            int p = rand()%(int)1e8+2;//range 2 to 10^8+1
            vec[i].first = p;
            vec[i].second = -1;

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
    if(p==2) res=true;
    if(p%2==0) res=false;
    for(int i=3;i*i<=p;i+=2){
        if(0==p%i){
            res=false;
            break;
        }
    }
    main.report(idx,res);

}   
};

#include "prime.def.h"