#include "oddeven.decl.h"

class Main : public CBase_Main {
  public:
    Main(CkArgMsg* msg);

};

class Elements : public CBase_Elements {
private:
    CProxy_Main mainProxy;
    int data;
    int state = 0;
    int bufferData = -1;
public:

    Elements(CProxy_Main &m){
        mainProxy = m;
        data = rand();
        if(thisIndex%2!=0){
            //begins by sending data to chare thisIndex-1
            sendStage1_oddidx();
        }else{
            //begins with waiting for the chare thisIndex+1
        }
    }

    void sendStage1_oddidx(){
        //send data to thisIndex - 1
        //go to stage B[wait for one of 2 paths]
        thisProxy(thisIndex - 1).recvStage1_evenidx(this->data);
        state = 1;
    }
    void recvStage1_oddidx(int data){
        //recv data from thisIndex - 1
        //change data if need be
        //go to stage C', wait for thisIndex + 1 to send for even stage
        /*
        may need to callback a buffered message from 2
        */
       if(state == 1){
            if(this->data < data){
                this->data = data;
            }
            state = 2;
       }
       else if(state == 3){
        //make the 0<->1 exchange if needed and send back to 2
        if(this->data < data){
            this->data = data;
        }
        int temp = -1;
        if(this->data > bufferData){
            this->data = bufferData;
        }
        thisProxy(thisIndex + 1).recvStage2_evenidx(temp);
       }
    }
    void recvStage2_oddidx(int data){
        //recv from thisIndex + 1
        //change if necessary
        //reply to thisIndex + 1
        /*
        may need to buffer this in case this arrives at stage B
        */
       if(state == 2){
            int origData = this->data;
            if(this->data > data){
                this->data = data;
            }
            thisProxy(thisIndex + 1).recvStage2_evenidx(origData);
            state = 0;
            // CkExit();
        //go to the next stage of odd even
       }
       if (state == 1){
        //buffer thisIndex + 1 data
        bufferData = data;
        state = 3;
       }
    }
    //part of some other func
    // void replyEvenStage_oddidx(){
    //     //send data to thisIndex + 1
    //     //go back to send Odd Stage
    // }
    void recvStage1_evenidx(int data){
        //recieve from thisIndex + 1
        //also reply for the stage and send for Even Stage
        int origData = this->data;
        if(this->data > data){
            this->data = data;
        }
        thisProxy(thisIndex + 1).recvStage1_oddidx(origData);
        if(thisIndex > 0){
            thisProxy(thisIndex - 1).recvStage2_oddidx(this->data);
        }
        state = 1;
    }
    void recvStage2_evenidx(int data){
        //go back to state A
        if(state == 1){
            if(this->data < data){
                this->data = data;
            }
            state = 0;
        }

    }
   

    

};
#include "oddeven.def.h"