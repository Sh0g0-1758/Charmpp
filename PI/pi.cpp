#include<iostream>
#include<random>
#include "piCalc.decl.h"

class Main : public CBase_Main{
private:
    int numTrials;
    int numInCircle=0;
    int count=0;
public:
    Main(CkArgMsg* m){
        if(m->argc!=3){
            ckout<<"usage: ./hello <numTrial> <numChare>"<<endl;
        }
        numTrials = atoi(m->argv[1]);
        int numChares = atoi(m->argv[2]);
        
        ckout<<"numTrials: "<<numTrials<<" numChares: "<<numChares<<endl;
        for(int i=0;i<numChares;i++){
            if(i==numChares-1){
                // ckout<<"creating chare with numTrials: "<<numTrials-i*numTrials/(numChares)<<endl;
                CProxy_Worker::ckNew(numTrials-i*(numTrials/(numChares)),thisProxy);
            }
            else{
                // ckout<<"creating chare with numTrials: "<<numTrials/(numChares)<<endl;
                CProxy_Worker::ckNew(numTrials/(numChares),thisProxy);
            }
        }
    }

    //takes the partial trials and aggregated them
    void done(int numTrials, int numInCircle){
        ckout<<"reported numcircle "<< numInCircle<<" numTrials "<<numTrials<<endl;
        this->count+=numTrials;
        this->numInCircle+=numInCircle;
        if(this->count==this->numTrials){
            ckout<<"approx pi is: "<<4*(double)this->numInCircle/(double)this->numTrials<<endl;
            ckout<<"total time taken: "<<CkWallTimer()<<endl;
            CkExit();
        }
    }
};

class Worker:public CBase_Worker{
public:
    Worker(int  numTrials,CProxy_Main m){
        // ckout<<"started worker with numTrials: "<<numTrials<<endl;
        int numInCircle = 0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0,1.0);
        for(int i=0;i<numTrials;i++){
            double x = dis(gen);
            double y = dis(gen);
            if((x*x + y*y) < 1){
                numInCircle++;
            }
        }

        m.done(numTrials, numInCircle);
        // ckout<<"called and shit"<<endl;
    }
};

#include "piCalc.def.h" 