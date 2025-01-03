#include <stdio.h>
#include <random>
#include "primal.decl.h"

#define GRAIN_SIZE 20000

int getRandom(int min, int max) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
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
            ckout << "Usage: " << m -> argv[0] << " <n>" << endl;
        }
        int n = atoi(m -> argv[1]);
        nums = (int*)malloc(n * sizeof(int));
        check = (int*)calloc(n, sizeof(int));
        tot = n / GRAIN_SIZE;
        begin = CkTimer();
        for(int i = 0; i < tot; i++) {
            int n_arr[GRAIN_SIZE];
            for(int j = 0; j < GRAIN_SIZE; j++) {
                n_arr[j] = getRandom(1, 100000000);
                nums[i * GRAIN_SIZE + j] = n_arr[j];
            }
            CProxy_test offload = CProxy_test::ckNew(n_arr, i, thisProxy);
        }
    }

    void result(int index, bool b[GRAIN_SIZE]) {
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
    test(int n_arr[GRAIN_SIZE], int index, CProxy_start parent) {
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
                    for(int i = 3; i <= (n / 2); i+=2) {
                        if(n % i == 0) {
                            prime = false;
                            break;
                        }
                    }
                    b_arr[j] = prime;
                }
            }
        }
        parent.result(index, b_arr);
    }
};


#include "primal.def.h"
