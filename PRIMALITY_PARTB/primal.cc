#include <stdio.h>
#include <random>
#include "primal.decl.h"

int getRandom(int min, int max) {
    std::mt19937_64 gen(42);
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
    int GRAIN_SIZE;
public:
    start(CkArgMsg* m) : curr(0) {
        if(m -> argc != 2) {
            ckout << "Usage: " << m -> argv[0] << " <n_primes>" << " <GRAIN_SIZE>" << endl;
        }
        int n = atoi(m -> argv[1]);
        GRAIN_SIZE = atoi(m -> argv[2]);
        nums = (int*)malloc(n * sizeof(int));
        check = (int*)calloc(n, sizeof(int));
        tot = n / GRAIN_SIZE;
        begin = CkTimer();
        for(int i = 0; i < tot; i++) {
            int n_arr[GRAIN_SIZE];
            for(int j = 0; j < GRAIN_SIZE; j++) {
                n_arr[j] = getRandom(100000000, 10000000000);
                nums[i * GRAIN_SIZE + j] = n_arr[j];
            }
            CProxy_test offload = CProxy_test::ckNew(n_arr, GRAIN_SIZE, i, thisProxy);
        }
    }

    void result(int index, bool b[], int GRAIN_SIZE) {
        for(int i = 0; i < GRAIN_SIZE; i++) {
            check[index * GRAIN_SIZE + i] = b[i];
        }
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
    test(int n_arr[], int GRAIN_SIZE, int index, CProxy_start parent) {
        bool b_arr[GRAIN_SIZE];
        for(int j = 0; j < GRAIN_SIZE; j++) {
            int n = n_arr[j];
            if(n <= 1) {
                b_arr[j] = false;
            } else {
                if(n % 2 == 0) {
                    b_arr[j] = n == 2 ? true : false;
                } else {
                    bool prime = true;
                    for(int i = 3; i * i <= n; i+=2) {
                        if(n % i == 0) {
                            prime = false;
                            break;
                        }
                    }
                    b_arr[j] = prime;
                }
            }
        }
        parent.result(index, b_arr, GRAIN_SIZE);
    }
};

#include "primal.def.h"
