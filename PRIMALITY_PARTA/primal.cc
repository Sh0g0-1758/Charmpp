#include <stdio.h>
#include <random>
#include "primal.decl.h"

int getRandom(int min, int max) {
    static int counter = 0;
    std::mt19937_64 gen(42 + counter++);
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

class start : public CBase_start {
private:
    int* check;
    int* nums;
    int tot;
    int curr;
    double begin = 0;
    double end = 0;
public:
    start(CkArgMsg* m) : curr(0) {
        if(m -> argc != 2) {
            ckout << "Usage: " << m -> argv[0] << " <n_primes>" << endl;
        }
        int n = atoi(m -> argv[1]);
        nums = (int*)malloc(n * sizeof(int));
        check = (int*)calloc(n, sizeof(int));
        tot = n;
        begin = CkTimer();
        for(int i = 0; i < tot; i++) {
            nums[i] = getRandom(100000000, 1000000000);
            CProxy_test offload = CProxy_test::ckNew(nums[i], i, thisProxy);
        }
    }

    void result(int index, bool b) {
        check[index] = b;
        curr++;
        if(curr == tot) {
            end = CkTimer();
            ckout << "Time: " << end - begin << " seconds" << endl;
            CkExit();
        }
    }
};

class test : public CBase_test {
public:
    test(int n, int index, CProxy_start parent) {
        bool b;
        if(n <= 1) {
            b = false;
        } else {
            if(n % 2 == 0) {
                b = n == 2 ? true : false;
            } else {
                bool prime = true;
                for(int i = 3; i * i <= n; i+=2) {
                    if(n % i == 0) {
                        prime = false;
                        break;
                    }
                }
                b = prime;
            }
        }
        parent.result(index, b);
    }
};

#include "primal.def.h"
