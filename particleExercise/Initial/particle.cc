#include "particle.decl.h"
#include <set>
#include <utility>
#include <vector>
#include <map>

double delta;
typedef struct {
    double x;
    double y;
} coordinate;

class Main: public CBase_Main{
    public:
        std::map<int, int> epchCount;
        Main(CkArgMsg* msg){
            int sizeX = 10;
            int sizeY = 10;
            delta = 1.f/(sizeX*1000);
            CProxy_ParticleGrid::ckNew(thisProxy, sizeX, sizeY, sizeX, sizeY);
        }
        void getTotal(int total){
            static int epoch = 0;
            
            ckout<<"epoch "<<epoch++<<" count: "<<total<<endl;
            if(epoch==1000){
                for(auto it=epchCount.begin();it!=epchCount.end();it++){
                    ckout<<"epoch "<<it->first<<" count: "<<it->second<<endl;
                }
                CkExit();
            }
        }
        void callback(int count, int epoch){
            if(epchCount.find(epoch)==epchCount.end()){
                epchCount[epoch] = count;
            }
            else{
                epchCount[epoch] += count;
            }
        }
};



class ParticleGrid: public CBase_ParticleGrid{
    CProxy_Main main;
    int recvCount = 0;
    int numPoints;
    int sizeX = -1;
    int sizeY = -1;
    double x1;
    double x2;
    double y1;
    double y2;
    int epoch = 0;
    int numCount = 0;
    std::vector <coordinate> coords;
    public:
    ParticleGrid(CProxy_Main m, int sizeX, int sizeY){
        //start the step'
        main = m;
        numPoints = rand()%10000;
        this->sizeX = sizeX;
        this->sizeY = sizeY;
        x1 = thisIndex.x*(1.f/sizeX);
        x2 = (thisIndex.x+1)*(1.f/sizeX);
        y1 = thisIndex.y*(1.f/sizeY);
        y2 = (thisIndex.y+1)*(1.f/sizeY);
        for(int i=0;i<numPoints;i++){
            double x = x1 + drand48()*(x2-x1);
            double y = y1 + drand48()*(y2-y1);
            coords.push_back(coordinate{x,y});
            numCount++;
        }
        step();
        
    }

    void step(){
        //4 sends
        epoch++;
        // numPoints = coords.size();
        CkCallback cb(CkReductionTarget(Main,getTotal),main);
        contribute(sizeof(int), &numPoints, CkReduction::sum_int, cb);


        
        std::vector<coordinate> sendUp;
        std::vector<coordinate> sendDown;
        std::vector<coordinate> sendLeft;
        std::vector<coordinate> sendRight;
        std::vector<coordinate> sendDiagonalUpRight;
        std::vector<coordinate> sendDiagonalUpLeft;
        std::vector<coordinate> sendDiagonalDownRight;
        std::vector<coordinate> sendDiagonalDownLeft; 

        for(auto it=coords.begin();it!=coords.end();){
            double x = it->x;
            double y = it->y;

            x+=delta*(drand48()-0.5);
            y+=delta*(drand48()-0.5);

            int idxX = floor(x/(1.f/sizeX));
            int idxY = floor(y/(1.f/sizeY));

            if(x<0) x+=1;
            if(y<0) y+=1;
            if(x>=1) x-=1;
            if(y>=1) y-=1;

            if(idxY > thisIndex.y && idxX == thisIndex.x){
                sendUp.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxY < thisIndex.y && idxX == thisIndex.x){
                sendDown.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX > thisIndex.x && idxY == thisIndex.y){
                sendRight.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX < thisIndex.x && idxY == thisIndex.y){
                sendLeft.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX > thisIndex.x && idxY > thisIndex.y){
                sendDiagonalUpRight.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX < thisIndex.x && idxY > thisIndex.y){
                sendDiagonalUpLeft.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX > thisIndex.x && idxY < thisIndex.y){
                sendDiagonalDownRight.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else if(idxX < thisIndex.x && idxY < thisIndex.y){
                sendDiagonalDownLeft.push_back(coordinate{x,y});
                it = coords.erase(it);
                numCount--;
            }
            else{
                it++;
            }

        }

        //send up
        int idxX = thisIndex.x;
        int idxY = thisIndex.y+1;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendUp.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendUp.size());
            for(int j=0;j<sendUp.size();j++){
                tmp[2*j]= sendUp[j].x;
                tmp[2*j+1] = sendUp[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendUp.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send down
        idxX = thisIndex.x;
        idxY = thisIndex.y-1;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendDown.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendDown.size());
            for(int j=0;j<sendDown.size();j++){
                tmp[2*j]= sendDown[j].x;
                tmp[2*j+1] = sendDown[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendDown.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send right
        idxX = thisIndex.x+1;
        idxY = thisIndex.y; 
        idxX = idxX>=sizeX ? 0 : idxX;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        if(!sendRight.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendRight.size());
            for(int j=0;j<sendRight.size();j++){
                tmp[2*j]= sendRight[j].x;
                tmp[2*j+1] = sendRight[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendRight.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send left
        idxX = thisIndex.x-1;
        idxY = thisIndex.y;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        if(!sendLeft.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendLeft.size());
            for(int j=0;j<sendLeft.size();j++){
                tmp[2*j]= sendLeft[j].x;
                tmp[2*j+1] = sendLeft[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendLeft.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send diagonal up right
        idxX = thisIndex.x+1;
        idxY = thisIndex.y+1;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendDiagonalUpRight.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendDiagonalUpRight.size());
            for(int j=0;j<sendDiagonalUpRight.size();j++){
                tmp[2*j]= sendDiagonalUpRight[j].x;
                tmp[2*j+1] = sendDiagonalUpRight[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendDiagonalUpRight.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send diagonal up left
        idxX = thisIndex.x-1;
        idxY = thisIndex.y+1;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendDiagonalUpLeft.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendDiagonalUpLeft.size());
            for(int j=0;j<sendDiagonalUpLeft.size();j++){
                tmp[2*j]= sendDiagonalUpLeft[j].x;
                tmp[2*j+1] = sendDiagonalUpLeft[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendDiagonalUpLeft.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send diagonal down right
        idxX = thisIndex.x+1;
        idxY = thisIndex.y-1;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendDiagonalDownRight.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendDiagonalDownRight.size());
            for(int j=0;j<sendDiagonalDownRight.size();j++){
                tmp[2*j]= sendDiagonalDownRight[j].x;
                tmp[2*j+1] = sendDiagonalDownRight[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendDiagonalDownRight.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        //send diagonal down left
        idxX = thisIndex.x-1;
        idxY = thisIndex.y-1;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        idxY = idxY < 0 ? sizeY-1 : idxY;
        if(!sendDiagonalDownLeft.empty()){
            double* tmp = (double*)malloc(sizeof(double)*2*sendDiagonalDownLeft.size());
            for(int j=0;j<sendDiagonalDownLeft.size();j++){
                tmp[2*j]= sendDiagonalDownLeft[j].x;
                tmp[2*j+1] = sendDiagonalDownLeft[j].y;
            }
            thisProxy(idxX,idxY).recv(tmp,2*sendDiagonalDownLeft.size());
        }
        else{
            thisProxy(idxX,idxY).recv(nullptr,0);
        }

        recv(nullptr,0);
        }
    void recv(double* coordsRecv, int size){
        recvCount++;
        for(int i=0;i<size;i+=2){
            coords.push_back(coordinate{coordsRecv[i],coordsRecv[i+1]});
            numCount++;
        }
        if(recvCount==9){
            recvCount=0;
            // coordsIncoming.clear();
            step();
            // CkExit();
        }
    }
};

#include "particle.def.h"