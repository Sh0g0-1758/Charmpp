#include <random>
#include "kmeans.decl.h"

struct ClusterData {
    int count = 0;
    double sum_x = 0.0;
    double sum_y = 0.0;
};

CkReductionMsg *SumClusterData(int nmsg, CkReductionMsg **msgs) {
    int k_clusters = msgs[0]->getSize() / sizeof(ClusterData);
    ClusterData sum_cluster[k_clusters];
    for(int i = 0; i < nmsg; i++) {
        CkAssert(msgs[i]->getSize() == sizeof(ClusterData) * k_clusters);
        ClusterData *msg = (ClusterData*)msgs[i]->getData();
        for(int j = 0; j < k_clusters; j++) {
            sum_cluster[j].count += msg[j].count;
            sum_cluster[j].sum_x += msg[j].sum_x;
            sum_cluster[j].sum_y += msg[j].sum_y;
        }
    }
    return CkReductionMsg::buildNew(sizeof(ClusterData) * k_clusters, sum_cluster);
}

CkReduction::reducerType SumClusterDataReductionType;
void register_custom(void) {
    SumClusterDataReductionType = CkReduction::addReducer(SumClusterData);
}

class start : public CBase_start {
private:
    int n; // number of points
    int k; // number of clusters
    int m; // length of the chare array
    double* kxarr;
    double* kyarr;
    CProxy_points pointsArray;
public:

    double gen_rand() {
        static int counter = 0;
        std::mt19937_64 gen(counter++);
        std::uniform_real_distribution<double> dis(0, 1);
        return dis(gen);
    }

    ~start() {
        free(kxarr);
        free(kyarr);
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

    void Update(CkReductionMsg *msg) {
        ClusterData *all_data = (ClusterData*)msg->getData();
        bool converged = true;
        double kx[k];
        double ky[k];
        for(int i = 0; i < k; i++) {
            if (all_data[i].count == 0) {
                kx[i] = kxarr[i];
                ky[i] = kyarr[i];
            } else {
                kx[i] = all_data[i].sum_x / all_data[i].count;
                ky[i] = all_data[i].sum_y / all_data[i].count;
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
    ~points() {
        free(xarr);
        free(yarr);
    }
    void assign(double kx[], double ky[], int k_clusters) {
        int start = thisIndex * size;
        int end = start + size;
        ClusterData clusters[k_clusters];
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
            clusters[cluster].count++;
            clusters[cluster].sum_x += xarr[i];
            clusters[cluster].sum_y += yarr[i];
        }
        CkCallback cbsum(CkReductionTarget(start, Update), startProxy);
        contribute(sizeof(ClusterData) * k_clusters, clusters, SumClusterDataReductionType, cbsum);
    }
};

#include "kmeans.def.h"
