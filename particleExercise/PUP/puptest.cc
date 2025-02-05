#include "puptest.h"
#include "puptest.decl.h"


Main::Main(CkArgMsg* msg){
    // ckout<<"Hello from Main"<<endl;
    CProxy_worker w = CProxy_worker::ckNew(thisProxy,2);
    std::vector<Coordinate> v(2);
    this->v = v;
    this->w = w;
    w.work(v);
}

void Main::step(){
    epoch++;
    if(epoch==100){
        CkExit();
    }
    else{
        // CkPrintf("Hello from result\n");
        w.work(v);
    }
}
// void Main::test(std::vector<Coordinate> &v){
//     CkPrintf("Hello from test\n");
// }

worker::worker(CProxy_Main main){
    usesAtSync = true;
    this->main = main;
    // CkPrintf("Hello from worker\n");
    ckout<<thisIndex<<"on PE "<< CkMyPe()<<endl;
}


// void worker::pup(PUP::er &p){
//     p|v;
// }
void worker::work(const std::vector<Coordinate> v){
    epoch++;
    // CkPrintf("Hello from work\n");
    this->v = v;
    for(auto &it : this->v){
        it.x = thisIndex;
        it.y = thisIndex+1;
    }
    if(epoch%10==0){
        // ckout<<"load balancing"<<endl;
        AtSync();
    }
    else{
        //load imbalance
        // if(thisIndex%10==0){
        // for(int i=0;i<1000;i++){
        //     for(auto i:this->v){
        //     // CkPrintf("x: %d, y: %d\n",i.x,i.y);
        //     i.x = thisIndex;
        //     i.y = thisIndex+1; 
        //     }
        // }
        // }
        // else{
        for(auto i:this->v){
            CkPrintf("x: %d, y: %d\n",i.x,i.y);
        }
        CkCallback cb(CkReductionTarget(Main,step),main);
        contribute(cb);
    }
}

void worker::ResumeFromSync(){
    // if(thisIndex==0){
    //     CkPrintf("Hello from ResumeFromSync\n");
    //     CkPrintf("pe %d\n",CkMyPe());
    // }
    for(auto i:this->v){
        CkPrintf("x: %d, y: %d\n",i.x,i.y);
    }
    CkCallback cb(CkReductionTarget(Main,step),main);
    contribute(cb);
}


