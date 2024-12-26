#include <stdio.h>
#include "area.decl.h"

class start : public CBase_start {
public:
    start(CkArgMsg *m) {
        double pi = 3.145;
        CProxy_calculate calc = CProxy_calculate::ckNew(pi);
        int num_elements = 10;
        for(int i = 1; i <= num_elements - 1; i++) {
            calc.findarea(i , -1);
        }
        calc.findarea(num_elements, num_elements);
    }
};

class calculate : public CBase_calculate {
private:
    double pi;
    int tot;
    int curr;
public:
    calculate(double y) : pi(y), tot(0), curr(0) {
        ckout << "CALC CONSTRUCTOR" << endl;
    }

    void findarea(double r, int total) {
        if(total != -1) tot = total;
        double area = pi * r * r;
        ckout << "Area of circle with radius " << r << " is " << area << endl;
        curr++;
        if (curr == tot) {
            CkExit();
        }
    }
};

#include "area.def.h"
