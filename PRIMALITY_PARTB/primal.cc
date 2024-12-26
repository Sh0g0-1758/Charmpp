#include<stdio.h>
#include<map>
#include<random>
#include "primal.decl.h"

#define GRAIN_SIZE 10

int getRandom(int min, int max) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

class start : public CBase_start {
private:
std::map<int, bool> primes;
int tot;
int curr;
public:
    start(CkArgMsg* m) : curr(0) {
        if(m -> argc != 2) {
            ckout << "Usage: " << m -> argv[0] << " <n>" << endl;
        }
        int n = atoi(m -> argv[1]);
        tot = n / GRAIN_SIZE;
        for(int i = 0; i < tot; i++) {
            int n_arr[GRAIN_SIZE];
            for(int j = 0; j < GRAIN_SIZE; j++) {
                n_arr[j] = getRandom(1, 100000000);
                primes[n_arr[j]] = false;
            }
            CProxy_test offload = CProxy_test::ckNew(n_arr, thisProxy);
        }
    }

    void result(int a[GRAIN_SIZE], bool b[GRAIN_SIZE]) {
        for(int i = 0; i < GRAIN_SIZE; i++) {
            primes[a[i]] = b[i];
        }
        curr++;
        if(curr == tot) {
            for(auto it = primes.begin(); it != primes.end(); it++) {
                if(it -> second) {
                    ckout << it -> first << " is prime" << endl;
                } else {
                    ckout << it -> first << " is not prime" << endl;
                }
            }
            CkExit();
        }
    }
};

class test : public CBase_test {
public:
    test(int n_arr[GRAIN_SIZE], CProxy_start parent) {
        bool b_arr[GRAIN_SIZE];
        for(int j = 0; j < GRAIN_SIZE; j++) {
            int n = n_arr[j];
            if(n <= 1) {
                b_arr[j] = false;
            } else {
                if(n % 2 == 0) {
                    b_arr[j] = false;
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
        parent.result(n_arr, b_arr);
    }
};


#include "primal.def.h"
