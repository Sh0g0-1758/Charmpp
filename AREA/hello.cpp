#include <stdio.h>
#include "hello.decl.h"

/* mainchare */
//part 1
// class Main : public CBase_Main {
// public:
//     Main(CkArgMsg* m) {
//         double pi = 3.1415;
//         int numArea = 10;
//         CProxy_Compute sim = CProxy_Compute::ckNew(pi, numArea);
//         for (int i = 1; i <= numArea; i++)
//             sim.findArea(i);
        
//     }
// };

// class Compute : public CBase_Compute {
// private:
//     double y;
//     int numArea;
//     int countArea = 0;
// public:
//     Compute(double pi,int numArea) {
//         y = pi;
//         this->numArea = numArea;
//         ckout << "Hello from a Compute chare running ona " << CkMyPe() << endl;
//     }

//     void findArea(double r) {
//         ckout<<"numArea: "<<numArea<<endl;  
//         ckout << "Area of a circle of radius " << r << " is " << y * r * r << endl;
//         countArea++;
//         if (countArea == numArea)
//         {
//             ckout<<"All areas calculated. Exiting..."<<endl;
//             CkExit();
//         }
        
//     }
// };

class Main : public CBase_Main {
public:
    Main(CkArgMsg* m) {
        double pi = 3.1415;
        int numArea = 10;
        CProxy_Compute sim = CProxy_Compute::ckNew(pi);
        for (int i = 1; i < numArea; i++)
            sim.findArea(i,-1);
        sim.findArea(numArea,numArea);
        
    }
};

class Compute : public CBase_Compute {
private:
    double y;
    int numArea=-1;
    int countArea = 0;
public:
    Compute(double pi) {
        y = pi;

        ckout << "Hello from a Compute chare running ona " << CkMyPe() << endl;
    }

    void findArea(double r,int numArea) {
        ckout << "Area of a circle of radius " << r << " is " << y * r * r << endl;
        countArea++;
        if(numArea == -1)
            return;
        if(numArea != -1 && this->numArea==-1){
            ckout<<"setting numArea: "<<numArea<<endl;
        } 
        this->numArea = numArea;
        if (countArea == numArea)
        {
            ckout<<"All areas calculated. Exiting..."<<endl;
            ckout<<CkWallTimer()<<endl;
            CkExit();
        }
        
    }
};

#include "hello.def.h"