#include <random>
#include "kmeans.decl.h"

class start : public CBase_start {
private:
    int n; // number of points
    int k; // number of clusters
    int m; // length of the chare array
    double* kxarr;
    double* kyarr;
    int* k_num;
    double* k_sum_x;
    double* k_sum_y;
    bool num_done = false;
    bool sum_done = false;
    CProxy_points pointsArray;
public:

    double gen_rand() {
        static int counter = 0;
        std::mt19937_64 gen(counter++);
        std::uniform_real_distribution<double> dis(0, 1);
        return dis(gen);
    }

    start(CkArgMsg* msg) {
        if(msg->argc != 4) {
            ckout << "Usage: " << msg->argv[0] << " <n> <k> <m>" << endl;
            CkExit();
        }
        n = atoi(msg->argv[1]);
        k = atoi(msg->argv[2]);
        m = atoi(msg->argv[3]);
        delete msg;
        k_num = (int*)malloc(k * sizeof(int));
        k_sum_x = (double*)malloc(k * sizeof(double));
        k_sum_y = (double*)malloc(k * sizeof(double));
        double xarr[n];
        double yarr[n];
        for(int i = 0; i < n; i++) {
            xarr[i] = gen_rand();
            yarr[i] = gen_rand();
        }
        pointsArray = CProxy_points::ckNew(thisProxy, xarr, yarr, n, (n / m), m);
        double kx[k];
        double ky[k];
        kxarr = (double*)malloc(k * sizeof(double));
        kyarr = (double*)malloc(k * sizeof(double));
        for(int i = 0; i < k; i++) {
            kx[i] = gen_rand();
            kxarr[i] = kx[i];
            ky[i] = gen_rand();
            kyarr[i] = ky[i];
        }
        pointsArray.assign(kx, ky, k);
    }

    void UpdateCounts(int knum[], int size) {
        for(int i = 0; i < k; i++) {
            k_num[i] = knum[i];
        }
        num_done = true;
        check_convergence();
    }

    void UpdateCoords(double ksum[], int size) {
        for(int i = 0; i < k; i++) {
            k_sum_x[i] = ksum[2 * i];
            k_sum_y[i] = ksum[(2 * i) + 1];
        }
        sum_done = true;
        check_convergence();
    }

    void check_convergence() {
        if(sum_done && num_done) {
            sum_done = false;
            num_done = false;
            bool converged = true;
            double kx[k];
            double ky[k];
            for(int i = 0; i < k; i++) {
                if (k_num[i] == 0) {
                    kx[i] = kxarr[i];
                    ky[i] = kyarr[i];
                } else {
                    kx[i] = k_sum_x[i] / k_num[i];
                    ky[i] = k_sum_y[i] / k_num[i];
                }
                double diff = ((kx[i] - kxarr[i]) * (kx[i] - kxarr[i])) + ((ky[i] - kyarr[i]) * (ky[i] - kyarr[i]));
                kxarr[i] = kx[i];
                kyarr[i] = ky[i];
                if(diff > 0.001) {
                    converged = false;
                }
            }
            if (converged) {
                for(int i = 0; i < k; i++) {
                    CkPrintf("Cluster %d: (%f, %f)\n", i, kx[i], ky[i]);
                }
                CkExit();
            } else {
                pointsArray.assign(kx, ky, k);
            }
        }
    }
};

class points : public CBase_points {
private:
    CProxy_start startProxy;
    double* xarr;
    double* yarr;
    int num;
    int size;
public:
    points(CProxy_start startProxy, double lxarr[], double lyarr[], int num, int size) : startProxy(startProxy), num(num), size(size) {
        xarr = (double*)malloc(num * sizeof(double));
        yarr = (double*)malloc(num * sizeof(double));
        for(int i = 0; i < num; i++) {
            xarr[i] = lxarr[i];
            yarr[i] = lyarr[i];
        }
    }
    void assign(double kx[], double ky[], int k_clusters) {
        int start = thisIndex * size;
        int end = start + size;
        int cntarr[k_clusters] = {0};
        double sumarr[k_clusters * 2] = {0.0};
        for(int i = start; i < end; i++) {
            double min_dist = 10; // numbers are between 0 and 1
            int cluster = -1;
            for(int j = 0; j < k_clusters; j++) {
                double dist = ((xarr[i] - kx[j]) * (xarr[i] - kx[j])) + ((yarr[i] - ky[j]) * (yarr[i] - ky[j]));
                if(dist < min_dist) {
                    min_dist = dist;
                    cluster = j;
                }
            }
            cntarr[cluster]++;
            sumarr[2 * cluster] += xarr[i];
            sumarr[(2 * cluster) + 1] += yarr[i];
        }
        CkCallback cbcnt(CkReductionTarget(start, UpdateCounts), startProxy);
        contribute(sizeof(int) * k_clusters, cntarr, CkReduction::sum_int, cbcnt);
        CkCallback cbsum(CkReductionTarget(start, UpdateCoords), startProxy);
        contribute(sizeof(double) * k_clusters * 2, sumarr, CkReduction::sum_double, cbsum);
    }
};

#include "kmeans.def.h"
