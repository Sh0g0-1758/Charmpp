#include<stdio.h>

#include "fibo.decl.h"

#define THRESHOLD 100

class start : public CBase_start {
private:

public:
    start(CkArgMsg* m) {
        if(m -> argc != 2) {
            ckout << "Usage: " << m -> argv[0] << " <n>" << endl;
        }
        int n = atoi(m -> argv[1]);
        CProxy_worker offload = CProxy_worker::ckNew(n, CProxy_worker(), true);
    }
};

class worker : public CBase_worker {
private:
    int n;
    CProxy_worker parent;
    int counter;
    int fib_result;
    bool root;
public:
    worker(int n, CProxy_worker par, bool r) : n(n), parent(par), counter(0), fib_result(0), root(r) {
        if (n > THRESHOLD) {
            CProxy_worker offload1 = CProxy_worker::ckNew(n - 1, thisProxy, false);
            CProxy_worker offload2 = CProxy_worker::ckNew(n - 2, thisProxy, false);
        } else {
            int a = 0, b = 1, c = 0;
            for(int i = 2; i <= n; i++) {
                c = a + b;
                a = b;
                b = c;
            }
            parent.result(c);
        }
    }

    void result(int c) {
        counter++;
        fib_result += c;
        if(counter == 2) {
            if (root) {
                ckout << "Fibonacci(" << n << ") = " << fib_result << endl;
                CkExit();
            } else {
                parent.result(fib_result);
            }
        }
    }
};

#include "fibo.def.h"
