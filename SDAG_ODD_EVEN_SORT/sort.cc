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
        result = (int*)malloc(n * sizeof(int));
        for(int i = 0; i < n; i++) {
            arr[i] = gen_rand();
        }
        pointsArray = CProxy_points::ckNew(thisProxy, arr, n, n);
    }

    void fini(int indx, int elem) {
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
                ckout << "Result: ";
                for(int i = 0; i < n; i++) {
                    ckout << result[i] << " ";
                }
                ckout << endl;
                ckout << "Expected: ";
                for(int i = 0; i < n; i++) {
                    ckout << arr[i] << " ";
                }
                ckout << endl;
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
    int curr_stage = 0;
    bool empty_buffer = true;
    std::pair<int, std::pair<int,int>> buffer;
    points_SDAG_CODE
public:
    points(CProxy_start startProxy, int arr[], int size) : startProxy(startProxy), size(size) {
        curr_elem = arr[thisIndex];
        if(thisIndex % 2 == 0) {
            thisProxy[thisIndex + 1].comm(thisIndex, curr_elem, curr_stage);
        }
        thisProxy[thisIndex].recv();
    }

    void process(int indx, int elem) {
        int odd_comm = curr_elem;
        int send_to_this_indx = thisIndex;
        if(indx < thisIndex) {
            if(elem > curr_elem) curr_elem = elem;
            send_to_this_indx--;
        } else {
            if(elem < curr_elem) curr_elem = elem;
            send_to_this_indx++;
        }

        if(thisIndex % 2 == 1) {
            thisProxy[send_to_this_indx].comm(thisIndex, odd_comm, curr_stage);
            // now odd are ready to proceed to the next stage
            curr_stage++;
            if(thisIndex == size - 1) curr_stage++;
        } else {
            // even chares are receiving a message, that means they are ready to proceed
            // to the next stage. 
            curr_stage++;
            // even chare must change who they interact with
            if(send_to_this_indx > thisIndex) send_to_this_indx = thisIndex - 1;
            else send_to_this_indx = thisIndex + 1;

            if(thisIndex == 0) {
                curr_stage++;
                if(curr_stage != size) {
                    thisProxy[1].comm(thisIndex, curr_elem, curr_stage);
                }
            } else {
                if(curr_stage != size) {
                    thisProxy[send_to_this_indx].comm(thisIndex, curr_elem, curr_stage);
                }
            }
        }

        if(!empty_buffer) {
            empty_buffer = true;
            process(buffer.first, buffer.second.first);
        } else {
            if(curr_stage == size) {
                startProxy.fini(thisIndex, curr_elem);
            } else thisProxy[thisIndex].recv();
        }
    }
};

#include "sort.def.h"
