#include "liveViz.h"
#include "particle.decl.h"
#include <pup_stl.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <random>
#include <utility>
#include <vector>

#define NUM_ITERATIONS 100
#define DIMENSION 100.0

class start : public CBase_start {
private:
  int num_elems_per_chare;
  int size_of_chare;
  int row_size;
  CProxy_boxes boxesArray;
  int chare_done = 0;
  int total_number_of_particles = 0;
  int stages_done = 10;

public:
  start(CkArgMsg *msg) {
    if (msg->argc != 3) {
      ckout << "Usage: " << msg->argv[0] << " <n> <k>" << endl;
      CkExit();
    }
    num_elems_per_chare = atoi(msg->argv[1]);
    size_of_chare = atoi(msg->argv[2]);
    delete msg;
    row_size = ceil(DIMENSION / size_of_chare);

    CkArrayOptions opts(row_size, row_size);
    boxesArray = CProxy_boxes::ckNew(thisProxy, num_elems_per_chare, row_size,
                                     size_of_chare, opts);
    
    // setup liveviz
    CkCallback cbk(CkIndex_boxes::colorme(0),boxesArray);
    liveVizConfig cfg(liveVizConfig::pix_color,true);
    liveVizInit(cfg,boxesArray,cbk, opts);

    boxesArray.start();
  }

  void init(int tot) { total_number_of_particles = tot; }

  void status(int tot) {
    if (tot != total_number_of_particles) {
      ckout << "[ERROR] Some particles were lost" << endl;
      CkExit();
    } else {
      ckout << "[STATUS] Done with stage " << stages_done << endl;
      stages_done+=10;
    }
  }

  void fini() {
    chare_done++;
    if (chare_done == row_size * row_size) {
      ckout << "[STATUS] Done with stage " << stages_done << endl;
      ckout << "[SUCCESS] All particles were accounted for" << endl;
      ckout << "[STATUS] Simulation finished" << endl;
      CkExit();
    }
  }
};

struct point {
  float x;
  float y;
  int color; // 0 for blue, 1 for red
};

inline void operator|(PUP::er &p, point &c) {
  p | c.x;
  p | c.y;
  p | c.color;
}

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
  int size_of_chare;

  int seed;

  int curr_stage = 0;

  CProxy_start startProxy;

public:
  void pup(PUP::er &p) {
    CBase_boxes::pup(p);
    p | row_size;
    p | target_recv;
    p | lowx;
    p | highx;
    p | lowy;
    p | highy;
    p | seed;
    p | curr_stage;
    p | store;
    p | buff_store;
    p | recv_count;
    p | to_send;
    p | startProxy;
    p | done_with_curr_stage;
    p | size_of_chare;
  }

  boxes(CkMigrateMessage *msg) {}

  float gen_rand(int start, int end) {
    std::mt19937_64 gen(seed);
    seed++;
    std::uniform_real_distribution<float> dis(start, end);
    return dis(gen);
  }

  bool direction() { return rand() % 2; }

#define RAND_UPDATE(t)                                                         \
  if (direction()) {                                                           \
    float temp_##t = store[j].t + static_cast<float>(                          \
                                      rand() / static_cast<float>(RAND_MAX));  \
    store[j].t = temp_##t > DIMENSION ? DIMENSION : temp_##t;                  \
  } else {                                                                     \
    float temp_##t = store[j].t - static_cast<float>(                          \
                                      rand() / static_cast<float>(RAND_MAX));  \
    store[j].t = temp_##t < 0.0 ? 0.0 : temp_##t;                              \
  }

  boxes(CProxy_start startProxy, int num_elems_per_chare, int row_size,
        int size_of_chare)
      : startProxy(startProxy), row_size(row_size), size_of_chare(size_of_chare) {
    usesAtSync = true;
    int num_elems;
    if (CkMyPe() % 5 == 0) {
      num_elems = num_elems_per_chare * 2;
    } else {
      num_elems = num_elems_per_chare;
    }
    lowx = thisIndex.x * size_of_chare;
    lowy = thisIndex.y * size_of_chare;
    highx = lowx + size_of_chare > DIMENSION ? DIMENSION : lowx + size_of_chare;
    highy = lowy + size_of_chare > DIMENSION ? DIMENSION : lowy + size_of_chare;
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
    // overloaded processors have 80% red particles and 20% blue particles
    // others have 50% blue particles and 50% red particles
    for (int i = 0; i < num_elems; i++) {
      if(CkMyPe() % 5 == 0) {
        if(i < ((num_elems * 4) / 5)) store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy), 1});
        else store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy), 0});
      } else {
        if(i < (num_elems / 2)) store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy), 1});
        else store.push_back({gen_rand(lowx, highx), gen_rand(lowy, highy), 0});
      }
    }

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
    if (curr_stage == NUM_ITERATIONS) {
      for (auto it : store) {
        if (it.x < 0.0 or it.x > DIMENSION or it.y < 0.0 or it.y > DIMENSION) {
          ckout << "[Error] Particle out of bounds" << endl;
          CkExit();
        }
      }
      startProxy.fini();
    } else if (curr_stage % 10 == 0) {
      // check correctness and load balance after every 10 stages
      int tot = store.size();
      CkCallback cbcnt(CkReductionTarget(start, status), startProxy);
      contribute(sizeof(int), &tot, CkReduction::sum_int, cbcnt);
      AtSync();
    } else {
      // start the next stage as the current chare has all the updated data
      // that it needs.
      start();
    }
  }

  void ResumeFromSync() { start(); }

  void colorme (liveVizRequestMsg *m) {
    unsigned char *intensity;
    intensity = new unsigned char[3 * size_of_chare * size_of_chare];

    int sx=thisIndex.x*size_of_chare;
    int sy=thisIndex.y*size_of_chare;
    int w=size_of_chare;
    int h=size_of_chare;
    
    // so we divide our store vector in a grid of size size_of_chare*size_of_chare
    // and decide the color of each pixel by studying whether it has more red or blue particles
    int color_pixel[size_of_chare*size_of_chare];
    int bracket = store.size()/(size_of_chare*size_of_chare);
    int box = 0;
    for(int i = 0; i < store.size(); i+=bracket) {
      if(box == size_of_chare*size_of_chare) break;
      int red = 0;
      int blue = 0;
      for(int j = 0; j < bracket; j++) {
        if(store[i+j].color == 1) {
          red++;
        } else {
          blue++;
        }
      }

      if(bracket != 0) {
        if(red > blue) {
          color_pixel[box] = 1;
        } else if (red < blue) {
          color_pixel[box] = 0;
        } else {
          if(box%2 == 0) {
            color_pixel[box] = 1;
          } else {
            color_pixel[box] = 0;
          }
        }
      } else {
        if(store[i].color == 1) {
          color_pixel[i] = 1;
        } else {
          color_pixel[i] = 0;
        }
      }
    box++;
    }
    for(int i=0;i<size_of_chare;++i){
      for(int j=0;j<size_of_chare;++j){

        int color = color_pixel[i*size_of_chare+j];
        	
        if(color == 1) { // red
          intensity[3*(i*w+j)+0] = 255; // RED component
          intensity[3*(i*w+j)+1] = 0; // GREEN component
          intensity[3*(i*w+j)+2] = 0; // BLUE component
        } else { // blue
          intensity[3*(i*w+j)+0] = 0; // RED component
          intensity[3*(i*w+j)+1] = 0; // GREEN component
          intensity[3*(i*w+j)+2] = 255; // BLUE component
        }
      }
    }
    
    // Draw a green border on right and bottom of this chare array's pixel buffer
    for(int i=0;i<h;++i){
      intensity[3*(i*w+w-1)+0] = 0;     // RED component
      intensity[3*(i*w+w-1)+1] = 255;   // GREEN component
      intensity[3*(i*w+w-1)+2] = 0;     // BLUE component
    }
    for(int i=0;i<w;++i){
      intensity[3*((h-1)*w+i)+0] = 0;   // RED component
      intensity[3*((h-1)*w+i)+1] = 255; // GREEN component
      intensity[3*((h-1)*w+i)+2] = 0;   // BLUE component
    }

    liveVizDeposit(m, sx,sy, w,h, intensity, this);
    free(intensity);
  }
};

#include "particle.def.h"
