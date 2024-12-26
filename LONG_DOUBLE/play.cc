#include<stdio.h>

#include "play.decl.h"

class start : public CBase_start {
public:
    start(CkArgMsg* m) {
        long double k = 3456.344134;
        ckout << "k = " << k << endl;
    }
};

#include "play.def.h"