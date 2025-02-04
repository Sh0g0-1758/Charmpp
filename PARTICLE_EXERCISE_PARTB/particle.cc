#include "particle.decl.h"
#include <map>
#include <random>
#include <utility>
#include <vector>
#include <ctime>
#include <cstdlib>

#define NUM_ITERATIONS 100

class start : public CBase_start {
private:
  int num_elems_per_chare;
  int size_of_chare;
  int row_size;
  CProxy_boxes boxesArray;
  int chare_done = 0;
  std::map<int, int> stage_check;
  int total_number_of_particles = 0;

public:
  start(CkArgMsg *msg) {
    if (msg->argc != 3) {
      ckout << "Usage: " << msg->argv[0] << " <n> <k>" << endl;
      CkExit();
    }
    num_elems_per_chare = atoi(msg->argv[1]);
    size_of_chare = atoi(msg->argv[2]);
    delete msg;
    row_size = ceil(100.0 / size_of_chare);

    boxesArray = CProxy_boxes::ckNew(thisProxy, num_elems_per_chare, row_size,
                                     size_of_chare, row_size, row_size);
    boxesArray.start();
  }

  void init(int tot) {
    total_number_of_particles = tot;
  }

  void status(int tot) {
    if(tot != total_number_of_particles) {
      ckout << "[Error] Some particles were lost" << endl;
      CkExit();
    }
  }

  void fini() {
    chare_done++;
    if (chare_done == row_size * row_size) {
      ckout << "[Success] All particles were accounted for" << endl;
      ckout << "Simulation finished" << endl;
      CkExit();
    }
  }
};

struct point {
  float x;
  float y;
};

class boxes : public CBase_boxes {
private:
  std::vector<point> store;
  std::map<int, std::vector<point>> buff_store;
  std::map<int, int> recv_count;
  int target_recv;
  std::map<std::pair<int, int>, std::vector<point>> to_send;

  int row_size;
  int lowx;
  int highx;
  int lowy;
  int highy;
  bool done_with_curr_stage = false;

  int seed;

  int curr_stage = 0;

  CProxy_start startProxy;

public:
  float gen_rand(int start, int end) {
    std::mt19937_64 gen(seed);
    seed++;
    std::uniform_real_distribution<float> dis(start, end);
    return dis(gen);
  }

  bool direction() { return rand() % 2; }

#define RAND_UPDATE(t)                                                         \
  if (direction()) {                                                           \
    float temp_##t = store[j].t +                                              \
                 static_cast<float>(rand() / static_cast<float>(RAND_MAX));    \
    store[j].t = temp_##t > 100.0 ? 100.0 : temp_##t;                          \
  } else {                                                                     \
    float temp_##t = store[j].t -                                              \
                 static_cast<float>(rand() / static_cast<float>(RAND_MAX));    \
    store[j].t = temp_##t < 0.0 ? 0.0 : temp_##t;                              \
  }

  boxes(CProxy_start startProxy, int num_elems_per_chare, int row_size,
        int size_of_chare)
      : startProxy(startProxy), row_size(row_size) {
    int num_elems;
    if(CkMyPe() % 5 == 0) {
        num_elems = num_elems_per_chare * 2;
    } else {
        num_elems = num_elems_per_chare;
    }
    lowx = thisIndex.x * size_of_chare;
    lowy = thisIndex.y * size_of_chare;
    highx = lowx + size_of_chare > 100.0 ? 100.0 : lowx + size_of_chare;
    highy = lowy + size_of_chare > 100.0 ? 100.0 : lowy + size_of_chare;
    // Check if point is on corner
    if ((thisIndex.x == 0 || thisIndex.x == row_size - 1) &&
        (thisIndex.y == 0 || thisIndex.y == row_size - 1)) {
      target_recv = 3;
    }
    // Check if point is on edge but not corner
    else if (thisIndex.x == 0 || thisIndex.x == row_size - 1 ||
             thisIndex.y == 0 || thisIndex.y == row_size - 1) {
      target_recv = 5;
    }
    // Interior points
    else {
      target_recv = 8;
    }
    // RANDOM SEED
    seed = thisIndex.x * row_size + thisIndex.y;
    for (int i = 0; i < num_elems; i++)
      store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy)});

    CkCallback cbcnt(CkReductionTarget(start, init), startProxy);
    contribute(sizeof(int), &num_elems, CkReduction::sum_int, cbcnt);
  }

  void start() {
    srand(time(0));
    for (int j = 0; j < store.size(); j++) {
      RAND_UPDATE(y);
      RAND_UPDATE(x);
    }

    for (auto it = store.begin(); it != store.end();) {
      if ((*it).x < lowx or (*it).x > highx or (*it).y < lowy or
          (*it).y > highy) {
        int newXindex = thisIndex.x;
        int newYindex = thisIndex.y;
        if ((*it).x < lowx) {
          newXindex--;
        } else if ((*it).x > highx) {
          newXindex++;
        }
        if ((*it).y < lowy) {
          newYindex--;
        } else if ((*it).y > highy) {
          newYindex++;
        }
        if (newXindex < 0 or newXindex >= row_size or newYindex < 0 or
            newYindex >= row_size) {
          it++;
          continue;
        }
        to_send[{newXindex, newYindex}].push_back((*it));
        it = store.erase(it);
      } else {
        ++it;
      }
    }

    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        if (i == 0 and j == 0) {
          continue;
        }
        if (thisIndex.x + i < 0 or thisIndex.x + i >= row_size or
            thisIndex.y + j < 0 or thisIndex.y + j >= row_size) {
          continue;
        }
        std::vector<point> data = to_send[{thisIndex.x + i, thisIndex.y + j}];
        float arr[2 * data.size()];
        for (int i = 0; i < data.size(); i++) {
          arr[2 * i] = data[i].x;
          arr[(2 * i) + 1] = data[i].y;
        }
        thisProxy(thisIndex.x + i, thisIndex.y + j)
            .receiver(curr_stage, arr, 2 * data.size());
      }
    }
    to_send.clear();
    done_with_curr_stage = true;
    if (recv_count[curr_stage] == target_recv) {
      update();
    }
  }

  void receiver(int stage, float data[], int size) {
    for (int i = 0; i < size; i += 2) {
      buff_store[stage].push_back({data[i], data[i + 1]});
    }
    recv_count[stage]++;
    if (done_with_curr_stage && recv_count[curr_stage] == target_recv) {
      update();
    }
  }

  void update() {
    done_with_curr_stage = false;
    for (auto it : buff_store[curr_stage]) {
      store.push_back(it);
    }
    buff_store[curr_stage].clear();
    recv_count[curr_stage] = 0;
    curr_stage++;
    if (curr_stage == 100) {
      for(auto it : store) {
        if(it.x < 0.0 or it.x > 100.0 or it.y < 0.0 or it.y > 100.0) {
          ckout << "[Error] Particle out of bounds" << endl;
          CkExit();
        }
      }
      startProxy.fini();
    } else if(curr_stage % 10 == 0) {
      // check correctness and load balance after every 10 stages
      int tot = store.size();
      CkCallback cbcnt(CkReductionTarget(start, status), startProxy);
      contribute(sizeof(int), &tot, CkReduction::sum_int, cbcnt);
      start();
    } else {
      // start the next stage as the current chare has all the updated data
      // that it needs.
      start();
    }
  }
};

#include "particle.def.h"
