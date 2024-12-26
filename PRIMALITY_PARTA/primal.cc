#include<stdio.h>
#include<map>
#include<random>
#include "primal.decl.h"

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
        tot = atoi(m -> argv[1]);
        for(int i = 0; i < tot; i++) {
            int temp = getRandom(1, 100000000);
            primes[temp] = false;
            CProxy_test offload = CProxy_test::ckNew(temp, thisProxy);
        }
    }

    void result(int a, bool b) {
        primes[a] = b;
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
    test(int n, CProxy_start parent) {
        if(n <= 1) {
            parent.result(n, false);
        } else {
            if(n % 2 == 0) {
                parent.result(n, false);
            } else {
                bool prime = true;
                for(int i = 3; i <= (n / 2); i+=2) {
                    if(n % i == 0) {
                        prime = false;
                        break;
                    }
                }
                parent.result(n, prime);
            }
        }
    }
};


#include "primal.def.h"
