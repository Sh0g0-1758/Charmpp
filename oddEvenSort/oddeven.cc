#include "oddeven.decl.h"
#include <vector>

class Main : public CBase_Main {
int crecv = 0;
int irecv = 0;
int numData = 1000000;
CProxy_Elements elements;
std::vector<int> sortedData;
std::vector<int> initData;
double start = -1;
double end = -1; 
  public:
    Main(CkArgMsg* msg){
        start = CkTimer();
        elements = CProxy_Elements::ckNew(thisProxy, numData, numData);
        sortedData.resize(numData);
        initData.resize(numData);
    }
    void callback(int data, int index){
        sortedData[index] = data;
        crecv++;
        if(crecv == numData){
            end = CkTimer();
            ckout<<" Time: "<<end - start<<endl;
            for(int i=0;i<numData;i++){
                if(sortedData[i] != initData[i]){
                    ckout<<"mismatch at "<< i << " : "<<sortedData[i] <<" , "<<initData[i]<<endl;
                    CkExit();
                }
            }
            ckout<<"\033[32m" <<"all test passed"<<"\033[32m" <<endl;
            CkExit();
        }
    }
    void getInitData(int data, int index){
        irecv++;
        initData[index] = data;
        if(irecv== numData){
            ckout<<"got all the init data, sorting"<<endl;
            std::sort(initData.begin(), initData.end());
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
        srand(thisIndex);
        data = rand();
        mainProxy.getInitData(data, thisIndex);
        // data = numData - thisIndex;
        this->numData = numData;
        // ckout<<"Data at "<<thisIndex<<" is "<<data<<endl;
        if(thisIndex%2!=0){
            //begins by sending data to chare thisIndex-1
            sendStage1_oddidx();
        }else{
            //begins with waiting for the chare thisIndex+1
            // but if it's the last index it will have to send at the beginning
            if(thisIndex == numData - 1){
                count++;
                state = 1;
                // sendStage2_evenidx();
            }
        }
    }

    // void printCount(){
    //     ckout<<count<<endl;
    // }

    void sendStage1_oddidx(){
        //send data to thisIndex - 1
        //go to stage B[wait for one of 2 paths]
        // ckout<<"here"<<endl;
        // ckout<<count<<endl;
        // if(count == numData){
        //     mainProxy.callback(this->data, thisIndex);
        //     return;
        // }
        // ckout<<"sent st1 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
        
        thisProxy(thisIndex - 1).recvStage1_evenidx(this->data);
        state = 1;
        // count += 2;
    }
    void recvStage1_oddidx(int data){
        //recv data from thisIndex - 1
        //change data if need be
        //go to stage C', wait for thisIndex + 1 to send for even stage
        /*
        may need to callback a buffered message from 2
        */
    //    ckout<< "Recieved st1 at " << thisIndex  << " from " << thisIndex-1 << " state "<<state << endl;
    //    ckout<<state<<endl;
    //DATA FROM BEHIND
       if(state == 1){
            count++;
            // ckout<<" index "<< thisIndex << " count "<<count<<endl;
            if(this->data < data){
                this->data = data;
                // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
            }
            if(thisIndex == numData - 1){
                count++;
                // ckout<<" index "<< thisIndex << " count "<<count<<endl;
                state = 0;
                if(count == numData){
                    mainProxy.callback(this->data, thisIndex);
                    return;
                }
                sendStage1_oddidx();
                return;
            }
            state = 2;
       }
       else if(state == 3){
        //make the 0<->1 exchange if needed and send back to 2
        count+=2;   
        // ckout<<" index "<< thisIndex << " count "<<count<<endl;

        if(this->data < data){
            // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
            this->data = data;
        }
        int temp = this->data;
        if(this->data > bufferData){
            // ckout<<"\t"<<thisIndex<<" set data to "<<bufferData<<endl;
            this->data = bufferData;
            // count++;
        }
        // ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
        thisProxy(thisIndex + 1).recvStage2_evenidx(temp);
       if(count == numData){
            mainProxy.callback(this->data, thisIndex);
            return;
        }
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
    //    ckout<< "Recieved st2 at " << thisIndex << " from " << thisIndex+1 << " state "<<state<<endl;
       if(state == 2){
            count++;
            // ckout<<" index "<< thisIndex << " count "<<count<<endl;
            int origData = this->data;
            if(this->data > data){
                // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
                this->data = data;
            }
            // ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
            thisProxy(thisIndex + 1).recvStage2_evenidx(origData);
            state = 0;
            if(count == numData){
                    mainProxy.callback(this->data, thisIndex);
                    return;
                }
            sendStage1_oddidx();
            // CkExit();
        //go to the next stage of odd even
       }
       else if (state == 1){
            //buffer thisIndex + 1 data
            bufferData = data;
            // ckout<<"\t\tBuffered data "<<bufferData<<endl;
            state = 3;
       }
    }

    void recvStage1_evenidx(int data){
        //recieve from thisIndex + 1
        //also reply for the stage and send for Even Stage
        // ckout<<count<<endl;
        // ckout<<"Recived st1 at "<<thisIndex<<" from "<<thisIndex + 1<<endl;
        // if(count == numData){
        //     // ckout<< "Finished sorting" << endl;
        //     // CkExit();
        //     mainProxy.callback(data, thisIndex);
        //     return;
            
        // }
        if(state == 0){
            // ckout<< "Recieved from " << thisIndex + 1 << " at " << thisIndex << endl;
            count+=1;
            // ckout<<" index "<< thisIndex << " count "<<count<<endl;
            int origData = this->data;
            if(this->data > data){
                // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
                this->data = data;
            }
            // ckout<<"sent st1 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
            thisProxy(thisIndex + 1).recvStage1_oddidx(origData);
            if(thisIndex == 0){
                count++;
                // ckout<<" index "<< thisIndex << " count "<<count<<endl;
                state = 0;
                if(count == numData){
                    mainProxy.callback(this->data, thisIndex);
                }
                return;
            }
            // ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
            thisProxy(thisIndex - 1).recvStage2_oddidx(this->data);
            state  = 1;
        }
        else if (state == 1){
            //buffer thisIndex + 1 data
            bufferData = data;
            state = 2;
        }
        if(count == numData){
            mainProxy.callback(this->data, thisIndex);
        }
        
    }

    void recvStage2_evenidx(int data){
        //recv from this index - 1 
        // ckout<<"Recived st2 at "<<thisIndex<<" from "<<thisIndex - 1<<endl;
        if(state == 1){
            count++;
            // ckout<<" index "<< thisIndex << " count "<<count<<endl;
            if(this->data < data){
                // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
                this->data = data;
            }
            state = 0;
        }
        else if (state == 2){
           count+=2;
        //    ckout<<" index "<< thisIndex << " count "<<count<<endl;
            // this->data = data;
            if(this->data < data){
                // ckout<<"\t"<<thisIndex<<" set data to "<<data<<endl;
                this->data = data;
            }
            int origData = this->data;
            if(this->data > bufferData){
                // ckout<<"\t"<<thisIndex<<" set data to "<<bufferData<<endl;
                
                this->data = bufferData;
                // ckout<<"\t"<<thisIndex<<" set data to "<<bufferData<<endl;
            }
            //send st1
            // ckout<<"sent st1 from "<<thisIndex <<" to "<<thisIndex + 1<<endl;
            thisProxy(thisIndex + 1).recvStage1_oddidx(origData);
            // ckout<<"sent st2 from "<<thisIndex <<" to "<<thisIndex - 1<<endl;
            thisProxy(thisIndex - 1).recvStage2_oddidx(this->data);
            state = 1;
        }
        if(count == numData){
            mainProxy.callback(this->data, thisIndex);
        }
    }
};
#include "oddeven.def.h"