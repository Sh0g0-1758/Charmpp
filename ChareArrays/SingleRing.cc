#include "singleRing.decl.h"

class Main : public CBase_Main{
public:
Main(CkArgMsg* m){
    if(m->argc!=3){
        CkPrintf("Usage: %s <ringSize> <trips>\n",m->argv[0]);
        CkExit();
    }
    
    int rs = atoi(m->argv[1]);
    int trips = atoi(m->argv[2]);

    CProxy_Ring r = CProxy_Ring::ckNew(rs,thisProxy,rs);

    CkPrintf("Starting ring with %d elements and %d trips\n",rs,trips);
    r[0].step(rs,trips,-1,-1);
}
void ringFinished(){
    CkExit();
}

void result(int result){
    CkPrintf("Result: %d\n",result);
}

};

class Ring : public CBase_Ring{
private:
int ringSize;
CProxy_Main main;
public:
    Ring(int rs, CProxy_Main m){
        ckout<<"Ring["<<thisIndex<<"] created on ("<<CkMyPe()<<")"<<endl;
        ringSize = rs;
        main = m;
       
    }

    void step(int elementLeft, int tripsLeft, int fromIndex, int fromPE){
        //step to the next;wrap around if elementLeft == 0
        // CkPrintf("Ring[%d] on (%d) received from Ring[%d] on (%d)\n",CkMyPe(),thisIndex,fromIndex,fromPE);
        ckout<<"Ring["<<thisIndex<<"] on ("<<CkMyPe()<<") received from Ring["<<fromIndex<<"] on ("<<fromPE<<") in trip "<<tripsLeft<<endl;
        CkCallback cb(CkReductionTarget(Main,result),main);
        contribute(sizeof(int),&tripsLeft,CkReduction::sum_int,cb);
        // CkCallback cb2(CkReductionTarget(Main,result),main);
        // contribute(sizeof(int),&tripsLeft,CkReduction::sum_int,cb2);
        //skip
        // if(elementLeft>1){
        //     //skis random number of elements as long as within the ring size
        //     int skip = rand()%elementLeft+1;
        //     if(skip==elementLeft) skip = elementLeft-1;
        //     thisProxy[(thisIndex+skip)%ringSize].step(elementLeft-skip,tripsLeft,thisIndex,CkMyPe());
        // }
        // else if(tripsLeft>1){
        //     thisProxy[(thisIndex+1)%ringSize].step(ringSize,tripsLeft-1,thisIndex,CkMyPe());
        // }
        // else{
        //     main.ringFinished();
        // }

        //no skip
        if(elementLeft>1){
            //skis random number of elements as long as within the ring size
            // int skip = rand()%elementLeft+1;
            // if(skip==elementLeft) skip = elementLeft-1;
            thisProxy[(thisIndex+1)%ringSize].step(elementLeft-1,tripsLeft,thisIndex,CkMyPe());
        }
        else if(tripsLeft>1){
            thisProxy[(thisIndex+1)%ringSize].step(ringSize,tripsLeft-1,thisIndex,CkMyPe());
        }
        else{
            main.ringFinished();
        }
    }   


};

#include "singleRing.def.h"