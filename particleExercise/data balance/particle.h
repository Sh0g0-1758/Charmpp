class coordinate;

#include "particle.decl.h"
#include <set>
#include <utility>
#include <vector>
#include <map>

class coordinate:public PUP::able{
    PUPable_decl(coordinate);

    coordinate():x(0),y(0){}
    coordinate(double x, double y):x(x),y(y){}
    coordinate(CkMigrateMessage *m):PUP::able(m){}


    virtual void pup(PUP::er &p){
        PUP::able::pup(p);
        p|x;
        p|y;
    }
    double x;
    double y;
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
    public:
    ParticleGrid(CProxy_Main m, int sizeX, int sizeY);
    ParticleGrid(CkMigrateMessage *m){}
    void pup(PUP::er &p);
    void step();
    void recv(double* coordsRecv, int size);
    void ResumeFromSync();
};
#include "particle.def.h"