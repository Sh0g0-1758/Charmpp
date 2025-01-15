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

    void updateMeans(CkReductionMsg* msg){
        reductionEle* ele = (reductionEle*)msg->getData();
        for(int i=0;i<k;i++){
            if(ele[i].num==0){
                continue;
            }
            kmeans[i*2] = ele[i].sumx/ele[i].num;
            kmeans[i*2+1] = ele[i].sumy/ele[i].num;
        }
    }

    void CheckAndStep(int numChangedAllegence){
        ckout<<numChangedAllegence<<endl;
        if(numChangedAllegence==0){
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
        delete[] counts;
    }
    
};

class Points : public CBase_Points{
private:
std::vector<std::pair<double,double>> points;
CProxy_Main main;
std::vector<reductionEle> kmeansLocal;
std::vector<int> oldAllegence;

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
        oldAllegence.resize(numPoints);


        for(int i=0;i<numPoints;i++){
            points[i].first = drand48();
            points[i].second = drand48();
            oldAllegence[i] = -1;
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
        int changedAllegence = 0;
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
            if(minIdx!=oldAllegence[i]){
                changedAllegence++;
                oldAllegence[i] = minIdx;
            }
            kmeansLocal[minIdx].num++;
            kmeansLocal[minIdx].sumx += points[i].first;
            kmeansLocal[minIdx].sumy += points[i].second;
        }
        
        CkCallback cb(CkReductionTarget(Main,updateMeans),main);
        contribute(k*sizeof(reductionEle),(reductionEle* )kmeansLocal.data(),kmeansReductionType,cb);
        ckout<<"Points["<<thisIndex<<"] finished assignment"<<endl;
        CkCallback cb2(CkReductionTarget(Main,CheckAndStep),main);
        contribute(sizeof(int),&changedAllegence,CkReduction::sum_int,cb2);
    }

};



#include "kmean.def.h"