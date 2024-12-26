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




/*
Hello sir!

We have gone through the 2 chapters you attached and also did the exercises given at the end of chapter 2 (primality was one of them). We have also done part A of the primality exercise and for part B, we need to learn about passing data arrays in chares, We are hoping you would give us some reading material on it. Also, a couple questions: 

1. Why does ckout not work with a long double.
2. I noticed that the first run usually takes longer time compared to subsequent runs. I found it off since for now the chares work on atomic data and thus there should be no performance benefit of data being already in the cache. 

That being said, We were particularly impressed by Charm++'s elegant runtime system, especially its intuitive approach to load balancing. The system's design, which uses chares as independent work units across processors, presents a compelling approach to parallel computing. We also wanted to apologize for the delay in communicating back to you. The past week was very hectic for both of us. I hope you understand. 

Looking forward to hear from you soon. 

Thank you
Shourya Goel
*/