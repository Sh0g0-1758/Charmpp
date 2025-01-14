#include <random>
#include "kmeans.decl.h"

struct ClusterData {
    int count = 0;
    double sum_x = 0.0;
    double sum_y = 0.0;
    bool* cluster_assignments;
    int num;
};

CkReductionMsg *SumClusterData(int nmsg, CkReductionMsg **msgs) {
    int k_clusters = msgs[0]->getSize() / sizeof(ClusterData);
    ClusterData sum_cluster[k_clusters];
    int num = ((ClusterData*)(msgs[0]->getData()))[0].num;
    for(int i = 0; i < k_clusters; i++) {
        sum_cluster[i].cluster_assignments = (bool*)malloc(num * sizeof(bool));
    }
    for(int i = 0; i < nmsg; i++) {
        CkAssert(msgs[i]->getSize() == sizeof(ClusterData) * k_clusters);
        ClusterData *msg = (ClusterData*)msgs[i]->getData();
        for(int j = 0; j < k_clusters; j++) {
            sum_cluster[j].count += msg[j].count;
            sum_cluster[j].sum_x += msg[j].sum_x;
            sum_cluster[j].sum_y += msg[j].sum_y;
            sum_cluster[j].num = num;
            for(int k = 0; k < num; k++) {
                sum_cluster[j].cluster_assignments[k] += msg[j].cluster_assignments[k];
            }
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
    int* cluster_assignments;
    CProxy_points pointsArray;
public:

    double gen_rand() {
        static int counter = 0;
        std::mt19937_64 gen(counter++);
        std::uniform_real_distribution<double> dis(0, 1);
        return dis(gen);
    }

    void initassignclusters() {
        cluster_assignments = (int*)malloc(n * sizeof(int));
        for(int i = 0; i < n; i++) {
            cluster_assignments[i] = -1;
        }
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
        initassignclusters();
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
            kxarr[i] = kx[i];
            kyarr[i] = ky[i];
            for(int j = 0; j < all_data[i].num; j++) {
                if(all_data[i].cluster_assignments[j] == true) {
                    if(cluster_assignments[j] != i) {
                        converged = false;
                    }
                    cluster_assignments[j] = i;
                }
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
    void assign(double kx[], double ky[], int k_clusters) {
        int start = thisIndex * size;
        int end = start + size;
        ClusterData clusters[k_clusters];
        for(int i = 0; i < k_clusters; i++) {
            clusters[i].cluster_assignments = (bool*)malloc(num * sizeof(bool));
            clusters[i].num = num;
        }
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
            clusters[cluster].cluster_assignments[i] = true;
            clusters[cluster].count++;
            clusters[cluster].sum_x += xarr[i];
            clusters[cluster].sum_y += yarr[i];
        }
        CkCallback cbsum(CkReductionTarget(start, Update), startProxy);
        contribute(sizeof(ClusterData) * k_clusters, clusters, SumClusterDataReductionType, cbsum);
    }
};

#include "kmeans.def.h"
