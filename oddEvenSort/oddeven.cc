#include "oddeven.decl.h"
#include <vector>

class Main : public CBase_Main {
int crecv = 0;
int numData = 3;
std::vector<int> sortedData;
  public:
    Main(CkArgMsg* msg){
        // int numData = 3;
        CProxy_Elements elements = CProxy_Elements::ckNew(thisProxy, numData, numData);
        sortedData.resize(numData);
    }
    void callback(int data, int index){
        ckout<<"Recieved data "<<data<<" from "<<index<<endl;
        sortedData[index] = data;
        crecv++;
        if(crecv == numData){
            for(int i = 0; i < numData; i++){
                ckout<<sortedData[i]<<endl;
            }
            CkExit();
        }
    }

};

class Elements : public CBase_Elements {
private:
    CProxy_Main mainProxy;
    int data;
    int state = 0;
    int bufferData = -1;
    int count = 0;
    int numData = 0;
public:

    Elements(CProxy_Main m, int numData){
        mainProxy = m;
        data = rand();
        this->numData = numData;
        if(thisIndex%2!=0){
            //begins by sending data to chare thisIndex-1
            sendStage1_oddidx();
        }else{
            //begins with waiting for the chare thisIndex+1
            // but if it's the last index it will have to send at the beginning
            if(thisIndex == numData - 1){
                state = 1;
                sendStage2_evenidx();
            }
        }
    }

    void sendStage1_oddidx(){
        //send data to thisIndex - 1
        //go to stage B[wait for one of 2 paths]
        ckout<<"here"<<endl;
        ckout<<"sent st1 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
        if(count == (numData/2)*2){
            // ckout<< "Finished sorting" << endl;
            // CkExit();
            mainProxy.callback(data, thisIndex);
        }
        thisProxy(thisIndex - 1).recvStage1_evenidx(this->data);
        state = 1;
        count += 2;
    }
    void recvStage1_oddidx(int data){
        //recv data from thisIndex - 1
        //change data if need be
        //go to stage C', wait for thisIndex + 1 to send for even stage
        /*
        may need to callback a buffered message from 2
        */
       ckout<< "Recieved st1 at " << thisIndex  << " from " << thisIndex-1 << endl;
       if(state == 1){
            if(this->data < data){
                this->data = data;
            }
            if(thisIndex == numData - 1){
                state = 0;
                sendStage1_oddidx();
            }
            state = 2;
       }
       else if(state == 3){
        //make the 0<->1 exchange if needed and send back to 2
        if(this->data < data){
            this->data = data;
        }
        int temp = this->data;
        if(this->data > bufferData){
            this->data = bufferData;
        }
        ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
        thisProxy(thisIndex + 1).recvStage2_evenidx(temp);
        sendStage1_oddidx();
       }
    }
    void recvStage2_oddidx(int data){
        //recv from thisIndex + 1
        //change if necessary
        //reply to thisIndex + 1
        /*
        may need to buffer this in case this arrives at stage B
        */
       ckout<< "Recieved st2 at " << thisIndex << " from " << thisIndex+1 << endl;
       if(state == 2){
            int origData = this->data;
            if(this->data > data){
                this->data = data;
            }
            thisProxy(thisIndex + 1).recvStage2_evenidx(origData);
            state = 0;
            sendStage1_oddidx();
            // CkExit();
        //go to the next stage of odd even
       }
       if (state == 1){
            //buffer thisIndex + 1 data
            bufferData = data;
            state = 3;
       }
    }

    void recvStage1_evenidx(int data){
        //recieve from thisIndex + 1
        //also reply for the stage and send for Even Stage
        ckout<<"Recived st1 at "<<thisIndex<<" from "<<thisIndex + 1<<endl;
        if(state == 0){
            // ckout<< "Recieved from " << thisIndex + 1 << " at " << thisIndex << endl;
            int origData = this->data;
            if(this->data > data){
                this->data = data;
            }
            thisProxy(thisIndex + 1).recvStage1_oddidx(origData);
            if(thisIndex == 0){
                state = 0;
                return;
            }
            ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
            thisProxy(thisIndex - 1).recvStage2_oddidx(this->data);
            state  = 1;
        }
        else if (state == 1){
            //buffer thisIndex + 1 data
            bufferData = data;
            state = 2;
        }
    }

    void recvStage2_evenidx(int data){
        //recv from this index - 1 
        ckout<<"Recived st2 at "<<thisIndex<<" from "<<thisIndex - 1<<endl;
        if(state == 1){
            if(this->data < data){
                this->data = data;
            }
            state = 0;
        }
        else if (state == 2){
           this->data = data;
            int temp = this->data;
            if(this->data > bufferData){
                this->data = bufferData;
            }
            //send st1
            ckout<<"sent st1 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
            thisProxy(thisIndex + 1).recvStage1_evenidx(temp);
            ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
            thisProxy(thisIndex - 1).recvStage2_oddidx(this->data);
            state = 1;
        }
    }
};
#include "oddeven.def.h"