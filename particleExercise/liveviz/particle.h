class coordinate;

#include "liveViz.h"
#include "particle.decl.h"
#include <set>
#include <utility>
#include <vector>
#include <map>

class coordinate:public PUP::able{
    PUPable_decl(coordinate);

    coordinate():x(0),y(0),color(0){}
    coordinate(double x, double y,int color):x(x),y(y),color(color){}
    coordinate(CkMigrateMessage *m):PUP::able(m){}


    virtual void pup(PUP::er &p){
        PUP::able::pup(p);
        p|x;
        p|y;
        p|color;
    }
    double x;
    double y;
    double color;
};

double delta;/*readonly*/

class Main:public CBase_Main{
    CProxy_ParticleGrid grid;
    public:
    Main(CkArgMsg* msg);
    void getTotal(int total);

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
    std::vector <coordinate> coords;
    unsigned char* intensity;
    public:
    ParticleGrid(CProxy_Main m, int sizeX, int sizeY);
    ParticleGrid(CkMigrateMessage *m){}
    void pup(PUP::er &p);
    void step();
    void recv(double* coordsRecv, int size);
    void ResumeFromSync();
    void getImage(liveVizRequestMsg* impl_msg);
};
#include "particle.def.h"