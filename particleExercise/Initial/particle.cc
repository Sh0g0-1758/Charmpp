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
                CkExit();
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


        std::map<std::pair<int,int>, std::vector<coordinate>> send;

        for(int i=thisIndex.x-1;i<=thisIndex.x+1;i++){
                for(int j = thisIndex.y-1;j<=thisIndex.y+1;j++){
                    if(i!=thisIndex.x || j!=thisIndex.y){
                        send[std::make_pair(i,j)] = std::vector<coordinate>();
                    }
            }
        }
        // if(thisIndex.x == 0 && thisIndex.y == 0){
        //     for(auto it:send){
        //         ckout<<"sending to "<<it.first.first<<" "<<it.first.second<<" "<<it.second.size()<<endl;
        //     }
        // }

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
            if(idxX !=thisIndex.x || idxY != thisIndex.y){
                send[std::make_pair(idxX,idxY)].push_back(coordinate{x,y});
                it = coords.erase(it);
            }
            else{
                it++;
            }
        }
        
        for(auto it:send){
            if(thisIndex.x==0 && thisIndex.y==0){
                // ckout<<"sending to "<<it.first.first<<" "<<it.first.second<<" "<<it.second.size()<<endl;
            }
            int idxX = it.first.first;
            int idxY = it.first.second;
            idxX = idxX>=sizeX ? 0 : idxX;
            idxX = idxX < 0 ? sizeX-1 : idxX;
            idxY = idxY>=sizeY ? 0 : idxY;
            idxY = idxY < 0 ? sizeY-1 : idxY;
            if(it.second.empty()){
                thisProxy(idxX,idxY).recv(nullptr,0);
            }
            else{
                double* tmp = (double*)malloc(sizeof(double)*2*it.second.size());
                for(int j=0;j<it.second.size();j++){
                    tmp[2*j]= it.second[j].x;
                    tmp[2*j+1] = it.second[j].y;
                }
                thisProxy(idxX,idxY).recv(tmp,2*it.second.size());
            }
        }
        recv(nullptr,0);
        }
    void recv(double* coordsRecv, int size){
        // ckout<<thisIndex.x<<" "<<thisIndex.y<<" "<<recvCount<<endl;
        recvCount++;
        for(int i=0;i<size;i+=2){
            coords.push_back(coordinate{coordsRecv[i],coordsRecv[i+1]});
            numCount++;
        }
        if(recvCount==9){
            recvCount=0;
            step();
        }
    }
};

#include "particle.def.h"