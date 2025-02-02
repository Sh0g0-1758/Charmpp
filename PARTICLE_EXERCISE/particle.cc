#include "particle.decl.h"
#include <map>
#include <random>
#include <utility>
#include <vector>

#define NUM_ITERATIONS 100

class start : public CBase_start {
private:
  int num_elems_per_chare;
  int size_of_chare;
  int row_size;
  CProxy_boxes boxesArray;
  int chare_done = 0;
  std::map<int, int> stage_check;

public:
  start(CkArgMsg *msg) {
    if (msg->argc != 3) {
      ckout << "Usage: " << msg->argv[0] << " <n> <k>" << endl;
      CkExit();
    }
    num_elems_per_chare = atoi(msg->argv[1]);
    size_of_chare = atoi(msg->argv[2]);
    delete msg;
    row_size = ceil(100 / size_of_chare);

    boxesArray = CProxy_boxes::ckNew(thisProxy, num_elems_per_chare, row_size,
                                     size_of_chare, row_size, row_size);
    boxesArray.start();
  }

  void status(int curr_stage, int Xindx, int Yindx, int max, int avg,
              int curr) {
    stage_check[curr_stage] += curr;
    ckout << "Chare [" << Xindx << ", " << Yindx << "] has " << curr
          << " particles. Over the last 10 iterations, Max: " << max
          << " Avg: " << avg << endl;
  }

  void fini() {
    chare_done++;
    if (chare_done == row_size * row_size) {
      bool particle_lost = false;
      for (auto it : stage_check) {
        if (it.second != num_elems_per_chare * row_size * row_size) {
          particle_lost = true;
          break;
        }
      }
      if (particle_lost) {
        ckout << "[Error] Some particles were lost" << endl;
      } else {
        ckout << "[Success] All particles were accounted for" << endl;
      }
      ckout << "Simulation finished" << endl;
      CkExit();
    }
  }
};

class boxes : public CBase_boxes {
private:
  int n;

  struct point {
    float x;
    float y;
  };

  std::vector<point> store;
  std::map<int, std::vector<point>> buff_store;
  std::map<int, int> recv_count;
  int target_recv;
  std::map<std::pair<int, int>, std::vector<point>> to_send;
  std::vector<int> state;

  int row_size;
  int lowx;
  int highx;
  int lowy;
  int highy;
  int size_of_chare;

  int seed;

  int curr_stage = 0;

  CProxy_start startProxy;

public:
  float gen_rand(int start, int end) {
    std::mt19937_64 gen(seed);
    seed += seed;
    std::uniform_real_distribution<float> dis(start, end);
    return dis(gen);
  }

  bool direction() { return (rand() / RAND_MAX > 0.5); }

#define RAND_UPDATE(t)                                                         \
  if (direction()) {                                                           \
    store[j].t = store[j].t +                                                  \
                 static_cast<float>(rand() / static_cast<float>(RAND_MAX));    \
    store[j].t > 100.0 ? 100.0 : store[j].t;                                   \
  } else {                                                                     \
    store[j].t = store[j].t -                                                  \
                 static_cast<float>(rand() / static_cast<float>(RAND_MAX));    \
    store[j].t < 0.0 ? 0.0 : store[j].t;                                       \
  }

  boxes(CProxy_start startProxy, int num_elems_per_chare, int row_size,
        int size_of_chare)
      : startProxy(startProxy), n(num_elems_per_chare), row_size(row_size),
        size_of_chare(size_of_chare) {
    state.push_back(n);
    lowx = thisIndex.x * size_of_chare;
    lowy = thisIndex.y * size_of_chare;
    highx = lowx + size_of_chare > 100.0 ? 100.0 : lowx + size_of_chare;
    highy = lowy + size_of_chare > 100.0 ? 100.0 : lowy + size_of_chare;
    // How many data messages each chare should receive.
    // Basically taking care of boundary chares.
    if (thisIndex.y == 0) {
      if (thisIndex.x == 0 or thisIndex.x == row_size - 1) {
        target_recv = 3;
      } else {
        target_recv = 5;
      }
    } else if (thisIndex.y == row_size - 1) {
      if (thisIndex.x == 0 or thisIndex.x == row_size - 1) {
        target_recv = 3;
      } else {
        target_recv = 5;
      }
    } else if (thisIndex.x == 0 and thisIndex.y != 0 and
               thisIndex.y != row_size - 1) {
      // CORNERS ARE ALREADY COVERED
      target_recv = 5;
    } else if (thisIndex.x == row_size - 1 and thisIndex.y != 0 and
               thisIndex.y != row_size - 1) {
      // CORNERS ARE ALREADY COVERED
      target_recv = 5;
    } else {
      target_recv = 8;
    }
    // RANDOM SEED
    seed = thisIndex.x * row_size + thisIndex.y + 42;
    for (int i = 0; i < n; i++)
      store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy)});
  }

  void start() {
    for (int j = 0; j < store.size(); j++) {
      RAND_UPDATE(x);
      RAND_UPDATE(y);
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
        float *arr = (float *)malloc(2 * data.size() * sizeof(float));
        for (int i = 0; i < data.size(); i++) {
          arr[i] = data[i].x;
          arr[i + 1] = data[i].y;
        }
        thisProxy(thisIndex.x + i, thisIndex.y + j)
            .receiver(curr_stage, arr, 2 * data.size());
      }
    }
    to_send.clear();
  }

  void receiver(int stage, float data[], int size) {
    for (int i = 0; i < size; i += 2) {
      buff_store[stage].push_back({data[i], data[i + 1]});
    }
    recv_count[stage]++;
    if (recv_count[curr_stage] == target_recv) {
      for (auto it : buff_store[curr_stage]) {
        store.push_back(it);
      }
      buff_store[curr_stage].clear();
      recv_count[curr_stage] = 0;
      curr_stage++;
      if (curr_stage % 10 == 0) {
        startProxy.status(curr_stage, thisIndex.x, thisIndex.y,
                          *std::max_element(state.begin(), state.end()),
                          std::accumulate(state.begin(), state.end(), 0.0) / 10,
                          state[9]);
        state.clear();
      }
      if (curr_stage == 100) {
        startProxy.fini();
      } else {
        // start the next stage as the current chare has all the updated data
        // that it needs.
        state.push_back(store.size());
        start();
      }
    }
  }
};

#include "particle.def.h"
