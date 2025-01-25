#include <random>
#include <utility>
#include "sort.decl.h"

class start : public CBase_start {
private:
    int n;
    int* arr;
    int* result;
    int curr = 0;
    CProxy_points pointsArray;
public:

    int gen_rand() {
        static int counter = 0;
        std::mt19937_64 gen(counter++);
        std::uniform_int_distribution<int> dis(1, 10000);
        return dis(gen);
    }

    start(CkArgMsg* msg) {
        if(msg->argc != 2) {
            ckout << "Usage: " << msg->argv[0] << " <size_of_arr>" << endl;
            CkExit();
        }
        n = atoi(msg->argv[1]);
        delete msg;
        arr = (int*)malloc(n * sizeof(int));
        result = (int*)calloc(n, sizeof(int));
        for(int i = 0; i < n; i++) {
            arr[i] = gen_rand();
        }
        pointsArray = CProxy_points::ckNew(thisProxy, arr, n, n);
    }

    void fini(int indx, int elem) {
        if(result[indx] != 0) return;
        result[indx] = elem;
        curr++;
        if(curr == n) {
            std::sort(arr, arr + n);
            bool correct = true;
            for(int i = 0; i < n; i++) {
                if(arr[i] != result[i]) {
                    correct = false;
                    break;
                }
            }
            if(correct) {
                ckout << "Correct Result" << endl;
            } else {
                ckout << "Incorrect Result" << endl;
            }
            CkExit();
        }
    }
};

class points : public CBase_points {
private:
    CProxy_start startProxy;
    int curr_elem;
    int size;
    bool prior_done = false;
    int buff = false;
    std::pair<int, int> buffer;
    int steps = 0;
public:
    points(CProxy_start startProxy, int arr[], int size) : startProxy(startProxy), size(size) {
        curr_elem = arr[thisIndex];
        if(thisIndex % 2 != 0) {
            thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
        }
    }

    void comm(int indx, int elem) {
        if(thisIndex % 2 == 0) {
            if(steps == 0 || thisIndex == 0) {
                int new_elem = curr_elem;
                if(curr_elem > elem) {
                    new_elem = elem;
                }
                thisProxy[indx].comm(thisIndex, curr_elem);
                curr_elem = new_elem;
                if(thisIndex != 0) thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                steps++;
                if(steps >= size) {
                    startProxy.fini(thisIndex, curr_elem);
                }
            } else {
                if(indx < thisIndex) {
                    if(buff) {
                        buff = false;
                        // coming from behind
                        if(curr_elem < elem) {
                            curr_elem = elem;
                        }
                        
                        // buffered message
                        int new_elem = curr_elem;
                        if(curr_elem > buffer.second) {
                            new_elem = buffer.second;
                        }
                        thisProxy[buffer.first].comm(thisIndex, curr_elem);
                        curr_elem = new_elem;
                        thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                        // two steps for even
                        steps+=2;
                        if(steps >= size) {
                            startProxy.fini(thisIndex, curr_elem);
                        }
                    } else {
                        if(curr_elem < elem) {
                            curr_elem = elem;
                        }
                        prior_done = true;
                        // one step for even
                        steps++;
                        if(steps >= size) {
                            startProxy.fini(thisIndex, curr_elem);
                        }
                    }
                } else {
                    if(prior_done) {
                        prior_done = false;
                        int new_elem = curr_elem;
                        if(curr_elem > elem) {
                            new_elem = elem;
                        }
                        thisProxy[indx].comm(thisIndex, curr_elem);
                        curr_elem = new_elem;
                        if(thisIndex != 0) thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                        // one step for even
                        steps++;
                        if(steps >= size) {
                            startProxy.fini(thisIndex, curr_elem);
                        }
                    } else {
                        buffer = std::make_pair(indx, elem);
                        buff = true;
                    }
                }
            }
        } else {
            if(indx < thisIndex) {
                if(buff) {
                    buff = false;
                    // coming from behind
                    if(curr_elem < elem) {
                        curr_elem = elem;
                    }
                    
                    // buffered message
                    int new_elem = curr_elem;
                    if(curr_elem > buffer.second) {
                        new_elem = buffer.second;
                    }
                    thisProxy[buffer.first].comm(thisIndex, curr_elem);
                    curr_elem = new_elem;
                    thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                    // two steps for odd
                    steps+=2;
                    if(steps >= size) {
                        startProxy.fini(thisIndex, curr_elem);
                    }
                } else {
                    if(curr_elem < elem) {
                        curr_elem = elem;
                    }
                    if(thisIndex == size - 1) {
                        thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                    } else {
                        prior_done = true;
                    }
                    // one step for odd
                    steps++;
                    if(steps >= size) {
                        startProxy.fini(thisIndex, curr_elem);
                    }
                }
            } else {
                if(prior_done) {
                    prior_done = false;
                    int new_elem = curr_elem;
                    if(curr_elem > elem) {
                        new_elem = elem;
                    }
                    thisProxy[indx].comm(thisIndex, curr_elem);
                    curr_elem = new_elem;
                    thisProxy[thisIndex - 1].comm(thisIndex, curr_elem);
                    // one step for odd
                    steps++;
                    if(steps >= size) {
                        startProxy.fini(thisIndex, curr_elem);
                    }
                } else {
                    buffer = std::make_pair(indx, elem);
                    buff = true;
                }
            }
        }
    }
};

#include "sort.def.h"
