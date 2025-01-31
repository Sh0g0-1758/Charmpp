#include "particle.decl.h"
#include <set>
#include <utility>
#include <vector>

double delta;
typedef struct {
    double x;
    double y;
} coordinate;

class Main: public CBase_Main{
    public:
        Main(CkArgMsg* msg){
            int sizeX = 10;
            int sizeY = 10;
            delta = (1.f/std::min(sizeX, sizeY))*(1.f/1000);
            CProxy_ParticleGrid::ckNew(thisProxy, sizeX, sizeY, sizeX, sizeY);
        }
        void getTotal(int total){
            static int epoch = 0;
            ckout<<"epoch "<<epoch++<<" count: "<<total<<endl;
        }
};

class ParticleGrid: public CBase_ParticleGrid{
    CProxy_Main main;
    int recvCount = 0;
    int numPoints = -1;
    int sizeX = -1;
    int sizeY = -1;
    double x1;
    double x2;
    double y1;
    double y2;
    std::vector <coordinate> coords;
    public:
    ParticleGrid(CProxy_Main m, int sizeX, int sizeY){
        //start the step'
        main = m;
        numPoints = rand()%1000;
        this->sizeX = sizeX;
        this->sizeY = sizeY;
        x1 = thisIndex.x*(1.f/sizeX);
        x2 = (thisIndex.x+1)*(1.f/sizeX);
        y1 = thisIndex.y*(1.f/sizeY);
        y2 = (thisIndex.y+1)*(1.f/sizeY);
        for(int i=0;i<numPoints;i++){
            double x = std::max(drand48()/sizeX + x1,x2);
            double y = std::max(drand48()/sizeY + y1,y2);
            coords.push_back(coordinate{x,y});
        }
        step();
    }
    int flattenIndex(int idxX, int idxY){
        int idxFlat = (idxX-thisIndex.x+1)*3 + (idxY-thisIndex.y+1);
        return idxFlat;
    }
    std::pair<int,int> unflattenIndex(int idxFlat){ 
        int idxX = idxFlat/3;
        int idxY = idxFlat%3;
        return std::make_pair(idxX,idxY);
    }
    void step(){
        //4 sends
        CkCallback cb(CkReductionTarget(Main,getTotal),main);
        //update coords and add to sending vecs
        // std::vector<std::pair<double,double>> sendUp;
        // std::vector<std::pair<double,double>> sendDown;
        // std::vector<std::pair<double,double>> sendRight;
        // std::vector<std::pair<double,double>> sendLeft;
        // std::vector<std::pair<double,double>> SendDiagUpRight;
        // std::vector<std::pair<double,double>> SendDiagUpLeft;
        // std::vector<std::pair<double,double>> SendDiagDownRight;
        // std::vector<std::pair<double,double>> SendDiagDownLeft;

        std::vector<std::vector<coordinate>> send(9);

        // auto it = coords.begin();
        for(auto it=coords.begin();it!=coords.end();it++){
            double x = it->x;
            double y = it->y;
            x+=delta*(drand48()-0.5);
            y+=delta*(drand48()-0.5);
            
            //calc new chare array index
            int idxX = static_cast<int>(x/(1.f/sizeX));
            int idxY = static_cast<int>(y/(1.f/sizeY));

            //loopback
            if(idxX != thisIndex.x || idxY != thisIndex.y){
                idxX = idxX>=sizeX ? 0 : idxX;
                idxY = idxY>=sizeY ? 0 : idxY;
                idxX = idxX < 0 ? sizeX-1 : idxX;
                idxY = idxY < 0 ? sizeY-1 : idxY;
                numPoints--;
            }
            int flatIdx = flattenIndex(idxX, idxY);
            send[flatIdx].push_back(coordinate{x,y});
        }
        for(int i=0;i<9;i++){
            if(i!=4){
                auto idx = unflattenIndex(i);
                thisProxy(idx.first,idx.second).recv(&send[i][0].x,2*send[i].size());
            }
            else{
                coords = send[i];
            }
        }
        }
    void recv(double* coordsRecv, int size){
        recvCount++;
        for(int i=0;i<size*2;i+=2){
            coords.push_back(coordinate{coordsRecv[i],coordsRecv[i+1]});
            numPoints++;
        }
        if(recvCount==8){
            recvCount=0;
            step();
        }
    }
};

#include "particle.def.h"