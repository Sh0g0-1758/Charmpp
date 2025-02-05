class Coordinate;

#include "puptest.decl.h"
#include <vector>

class Coordinate:public PUP::able{
    PUPable_decl(Coordinate);

    Coordinate():x(0),y(0){}
    Coordinate(double x, double y):x(x),y(y){}
    Coordinate(CkMigrateMessage *m):PUP::able(m){}

    virtual void pup(PUP::er &p){
        PUP::able::pup(p);
        p|x;
        p|y;
    }
    double x;
    double y;
};

class Main:public CBase_Main{
    std::vector<Coordinate> v;
    CProxy_worker w;
    int epoch = 0;
    public:
        Main(CkMigrateMessage *m){}
        Main(CkArgMsg *msg);

        void step();

};

class worker:public CBase_worker{
    public:
    std::vector<Coordinate> v;
    int epoch = 0;
    CProxy_Main main;
    worker(CProxy_Main m);
    worker(CkMigrateMessage *m){}
    void work(std::vector<Coordinate> v);
    void ResumeFromSync();
    void pup(PUP::er &p){
        p|v;
        p|epoch;
        p|main;
    }
    // private:
};

#include "puptest.def.h"