#include "kmean.decl.h"
#include <vector>
#include <utility>

struct reductionEle{
    double sumx=0.0;
    double sumy=0.0;
    int num=0;
};

CkReductionMsg *kmeansReduction(int nMsg,CkReductionMsg **msgs)
{
    int k=-1;
    std::vector<reductionEle> ret;
    for (int i=0;i<nMsg;i++) {
        // Sanity check:
        if(k==-1){
            k=msgs[i]->getSize()/sizeof(reductionEle);
            ret.resize(k);
        }
        else{
            CkAssert(msgs[i]->getSize()==k*sizeof(reductionEle));
        }
        // Extract this message's data
        reductionEle *m=(reductionEle *)msgs[i]->getData();
        for(int j=0;j<k;j++){
            ret[j].sumx += m[j].sumx;
            ret[j].sumy += m[j].sumy;
            ret[j].num += m[j].num;
        }
    }
    return CkReductionMsg::buildNew(k*sizeof(reductionEle),(reductionEle*)ret.data());
}


CkReduction::reducerType kmeansReductionType;

void registerRedKmeans(void)
{
  kmeansReductionType=CkReduction::addReducer(kmeansReduction);
}

class Main : public CBase_Main{
private:
    double *kmeans;
    double *kmeansNew;
    int* counts;
    int k;
    CProxy_Points p;
public:
    Main(CkArgMsg* m){
        if(m->argc!=4){
            CkPrintf("Usage: %s <numDataPoints> <numParallel> <numClusters>\n",m->argv[0]);
            CkExit();
        }
        
        int M = atoi(m->argv[1]);
        int N = atoi(m->argv[2]);
        int K = atoi(m->argv[3]);
        srand(42);

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
        
        // p.reductionTest(K);
        p.Assign(kmeans,K);
    }


    void UpdateAndStep(CkReductionMsg* msg){
        reductionEle* ele = (reductionEle*)msg->getData();
        for(int i=0;i<k;i++){
            kmeansNew[i*2] = ele[i].sumx;
            kmeansNew[i*2+1] = ele[i].sumy;
            counts[i] = ele[i].num;
        }
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
        }
    
    }

    void kmeansReductionTets(CkReductionMsg* msg){
        reductionEle* ele = (reductionEle*)msg->getData();
        for(int i=0;i<k;i++){
            ckout<<ele[i].sumx<<" "<<ele[i].sumy<<" "<<ele[i].num<<endl;
        }
        
        delete msg;
        CkExit();
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
std::vector<reductionEle> kmeansLocal;
public:
    void reductionTest(int k){
        reductionEle* ele = new reductionEle[k];
        for(int i=0;i<k;i++){
            ele[i].sumx = drand48();
            ele[i].sumy = drand48();
            ele[i].num = 2;
        }
        CkCallback cb(CkReductionTarget(Main,kmeansReductionTets),main);
        contribute(k*sizeof(reductionEle),ele,kmeansReductionType,cb);
        ckout<<"points["<<thisIndex<<"] finished reductionTest"<<endl;
    }

    Points(int M,int N,int K, CProxy_Main main){
        //Make points and buffers
        CkPrintf("Worker created on PE %d\n", CkMyPe());
        this->main = main;
        int numPoints = M/N;
        if(thisIndex == 0){
            //this is an unbalanced assignment can be improved
            numPoints += M%N;//ignorable if m>>n
        }

        points.resize(numPoints);
        kmeansLocal.resize(K);

        for(int i=0;i<numPoints;i++){
            points[i].first = drand48();
            points[i].second = drand48();
        }
    }


    void Assign(double kmeans[], int k){
        ckout<<"Points["<<thisIndex<<"] received new kmeans"<<endl;
        //zero out coord and count
        for(int i=0;i<k;i++){
            kmeansLocal[i].sumx = 0;
            kmeansLocal[i].sumy = 0;
            kmeansLocal[i].num = 0;
        }
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
            kmeansLocal[minIdx].num++;
            kmeansLocal[minIdx].sumx += points[i].first;
            kmeansLocal[minIdx].sumy += points[i].second;
        }
        
        CkCallback cb(CkReductionTarget(Main,UpdateAndStep),main);
        contribute(k*sizeof(reductionEle),(reductionEle* )kmeansLocal.data(),kmeansReductionType,cb);
        ckout<<"Points["<<thisIndex<<"] finished assignment"<<endl;
    }

};



#include "kmean.def.h"