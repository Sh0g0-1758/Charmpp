#include "prime.decl.h"
#include <vector>

//part A
class Main : public CBase_Main{
int totalPrime = 0;
int donePrime = 0;
int batchSize = 0;
double begin = 0;
double end = 0;
std::vector<std::pair<int,int>> vec;
public:
    Main(CkArgMsg* m){
        if(m->argc!=3){
            ckout<<"usage: ./prime <numPrime> <batchSize>"<<endl;
        }  
        srand(time(NULL));//for reproducibility
        totalPrime = atoi(m->argv[1]); 
        batchSize = atoi(m->argv[2]); 
        if(batchSize==-1){//using inf batch size
            batchSize = totalPrime;
        }
        vec.resize(totalPrime);
        begin = CkTimer();
        for(int i=0;i<totalPrime;i+=batchSize){
            int batch[std::min(batchSize,totalPrime-i)];//M size array for worker
            for(int j=0;j<std::min(batchSize,totalPrime-i);j++){
                int p = rand();
                vec[i+j].first = p;
                vec[i+j].second = -1;
                batch[j] = p;
            }
            ckout<<"Creating Worker for batch "<<i<<endl;
            CProxy_Worker::ckNew(batch,i,std::min(batchSize,totalPrime-i),thisProxy);
            
        }
    }
    void report(int p[],int idx,int size){
        ckout<<"Received report for batch "<<idx<<endl;
        for(int j=0;j<size;j++){
            vec[idx+j].second = p[j];
        }
        donePrime+=size;
        if(donePrime==totalPrime){
            end = CkTimer();
            // for(auto it:vec){
            //     ckout<<it.first<<" "<<it.second<<endl;
            // }
            // ckout<<"Time taken: "<<end-begin<<endl;
            ckout<<"time, "<<end-begin<<endl;
            CkExit();
        }
    }
};

bool isPrime(int p){
    if(p<=1) return false;
    if(p==2) return true;
    if(p%2==0) return false;
    for(int i=3;i*i<=p;i+=2){
        if(0==p%i){
            return false;
        }
    }
    return true;
}

class Worker : public CBase_Worker{
public:
Worker(int batch[],int idx,int size,CProxy_Main main){
    
    bool res=true;
    for(int j=0;j<size;j++){
      
        batch[j] = isPrime(batch[j]);
    }
    main.report(batch,idx,size);
}   
};

#include "prime.def.h"