#include "kmean.decl.h"
#include <vector>
#include <utility>

class Main : public CBase_Main{
private:
    double *kmeans;
    double *kmeansNew;
    int* counts;
    int k;
    CProxy_Points p;
    int calls = 0;//call to step
public:
    Main(CkArgMsg* m){
        if(m->argc!=4){
            CkPrintf("Usage: %s <numDataPoints> <numParallel> <numClusters>\n",m->argv[0]);
            CkExit();
        }
        
        int M = atoi(m->argv[1]);
        int N = atoi(m->argv[2]);
        int K = atoi(m->argv[3]);
        srand(time(NULL));

        kmeans = new double[2*K];
        kmeansNew = new double[2*K];
        counts = new int[K];
        k = K;
        p = CProxy_Points::ckNew(M,N,K,thisProxy,N);

        for(int i=0;i<K;i++){
            kmeans[i*2] = drand48();
            kmeans[i*2+1] = drand48();
        }

        ckout<<"Starting kmeans with "<<M<<" data points, "<<N<<" parallel executions and "<<K<<" clusters"<<endl;
        
       
        p.Assign(kmeans,K);

    }

    void UpdateCoords(int size,double coords[]){
        //Update coordinates
        ckout<<"Main received new coords"<<endl;
        for(int i=0;i<k;i++){
            kmeansNew[i*2] = coords[i*2];
            kmeansNew[i*2+1] = coords[i*2+1];
        }
        step();
    }
    void UpdateCounts(int counts[], int size){
        //Update counts
        ckout<<"Main received new counts"<<endl;
        for(int i=0;i<size;i++){
            this->counts[i] = counts[i];
        }
        step();
    }
    void step(){
        calls++;
        
        if(calls==2){
            CkPrintf("Main stepping\n");
            double diff = 0;
            for(int i=0;i<k;i++){
                if(counts[i]==0){
                    kmeansNew[i*2] = drand48();
                    kmeansNew[i*2+1] = drand48();
                    continue;
                }
                kmeansNew[i*2] = kmeansNew[i*2]/counts[i];
                kmeansNew[i*2+1] = kmeansNew[i*2+1]/counts[i];
            }
            
            for(int i=0;i<k;i++){
                diff += (kmeans[i*2] - kmeansNew[i*2])*(kmeans[i*2] - kmeansNew[i*2]) + (kmeans[i*2+1] - kmeansNew[i*2+1])*(kmeans[i*2+1] - kmeansNew[i*2+1]);
            }
            ckout<<diff<<endl;
            for(int i=0;i<k;i++){
                kmeans[i*2] = kmeansNew[i*2];
                kmeans[i*2+1] = kmeansNew[i*2+1];
            }
            if(diff < 1e-6){
                CkPrintf("Kmeans converged\n");
                CkExit();
            }
            else{
                
                p.Assign(kmeans,k);
                calls=0;
            }
        }
    }
    ~Main(){
        delete[] kmeans;
        delete[] kmeansNew;
        delete[] counts;
    }
    
};

class Points : public CBase_Points{
private:
std::vector<std::pair<double,double>> points;
CProxy_Main main;
std::vector<double> coord;
std::vector<int> count; 
public:
    Points(int M,int N,int K, CProxy_Main main){
        //Make points and buffers
        CkPrintf("Worker created on PE %d\n", CkMyPe());
        this->main = main;
        int numPoints = M/N;
        if(thisIndex == 0){
            //this is an unbalanced assignment can be improved
            numPoints += M%N;
        }

        points.resize(numPoints);
        coord.resize(K*2);
        count.resize(K);

        for(int i=0;i<numPoints;i++){
            points[i].first = drand48();
            points[i].second = drand48();
        }
    }

    void Assign(double kmeans[], int k){
        ckout<<"Points["<<thisIndex<<"] received new kmeans"<<endl;
        for(int i=0;i<points.size();i++){
            double minDist = 1e9;
            int minIdx = -1;
            for(int j=0;j<k;j++){
                double dist = (points[i].first - kmeans[j*2])*(points[i].first - kmeans[j*2]) + (points[i].second - kmeans[j*2+1])*(points[i].second - kmeans[j*2+1]);
                if(dist < minDist){
                    minDist = dist;
                    minIdx = j;
                }
            }
            count[minIdx]++;
            coord[minIdx*2] += points[i].first;
            coord[minIdx*2+1] += points[i].second;
        }

        CkCallback cb(CkReductionTarget(Main,UpdateCoords),main);
        contribute(2*k*sizeof(double), (double* )coord.data(), CkReduction::sum_double, cb);
        CkCallback cbcount(CkReductionTarget(Main,UpdateCounts),main);
        contribute(k*sizeof(int), (int *)count.data(), CkReduction::sum_int, cbcount);

        
        

        ckout<<"Points["<<thisIndex<<"] finished assignment"<<endl;
    }

};



#include "kmean.def.h"