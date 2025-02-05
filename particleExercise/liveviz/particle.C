#include "particle.h"
// #include "particle.decl.h"

#define numPix 100

Main::Main(CkArgMsg* msg){
    int sizeX = 10;
    int sizeY = 10;
    delta = 1.f/(sizeX*30);
    CkArrayOptions opts(sizeX, sizeY);
    grid = CProxy_ParticleGrid::ckNew(thisProxy, sizeX, sizeY, opts);

    // setup liveviz
    CkCallback c(CkIndex_ParticleGrid::getImage(0),grid);
    liveVizConfig cfg(liveVizConfig::pix_color,true);
    liveVizInit(cfg,grid,c, opts);
    grid.step();
    }

void Main::getTotal(int total){
    static int epoch = 0;
    
    ckout<<"epoch "<<++epoch<<" count: "<<total<<endl;
    if(epoch==10000){
        CkExit();
    }
    else{
        grid.step();
    }
}

/**************************************************************/

ParticleGrid::ParticleGrid(CProxy_Main m, int sizeX, int sizeY){

    main = m;
    int numRed = 0;
    if(CmiMyPe()%5==0){
        numPoints = rand()%50000;
        numRed = 0.9*numPoints;
    }
    else{
        numPoints = rand()%10000;
        numRed = 0.1*numPoints;
    }
    this->sizeX = sizeX;
    this->sizeY = sizeY;
    x1 = thisIndex.x*(1.f/sizeX);
    x2 = (thisIndex.x+1)*(1.f/sizeX);
    y1 = thisIndex.y*(1.f/sizeY);
    y2 = (thisIndex.y+1)*(1.f/sizeY);
    intensity = new unsigned char[3*numPix*numPix];
    for(int i=0;i<numPoints;i++){
        
        double x = x1 + drand48()*(x2-x1);
        double y = y1 + drand48()*(y2-y1);
        if(i<numRed){
            coords.push_back(coordinate{x,y,1});
        }
        else{
            coords.push_back(coordinate{x,y,0});
        }
    }
    usesAtSync = true;
}

void ParticleGrid::pup(PUP::er &p){
    p|main;
    p|recvCount;
    p|numPoints;
    p|sizeX;
    p|sizeY;
    p|x1;
    p|x2;
    p|y1;
    p|y2;
    p|epoch;
    p|coords;
}

void ParticleGrid::step(){
    std::map<std::pair<int,int>, std::vector<coordinate>> send;

    for(int i=thisIndex.x-1;i<=thisIndex.x+1;i++){
            for(int j = thisIndex.y-1;j<=thisIndex.y+1;j++){
                if(i!=thisIndex.x || j!=thisIndex.y){
                    send[std::make_pair(i,j)] = std::vector<coordinate>();
                }
        }
    }

    for(auto it=coords.begin();it!=coords.end();){
        double x = it->x;
        double y = it->y;
        if(abs(it->color-1)<1e-3){
        x+=2*delta*(drand48()-0.5);
        y+=2*delta*(drand48()-0.5);
        }
        else{
        x+=delta*(drand48()-0.5);
        y+=delta*(drand48()-0.5);
        }

        int idxX = floor(x/(1.f/sizeX));
        int idxY = floor(y/(1.f/sizeY));


        if(x<0) x=0.999999;
        if(y<0) y=0.999999;
        if(x>=1) x=0.000001;
        if(y>=1) y=0.000001;

        
        if(idxX !=thisIndex.x || idxY != thisIndex.y){
            send[std::make_pair(idxX,idxY)].push_back(coordinate{x,y,it->color});
            it = coords.erase(it);
        }
        else{
            it->x = x;
            it->y = y;
            it++;
        }
    }

    for(auto it:send){
        int idxX = it.first.first;
        int idxY = it.first.second;
        idxX = idxX>=sizeX ? 0 : idxX;
        idxX = idxX < 0 ? sizeX-1 : idxX;
        idxY = idxY>=sizeY ? 0 : idxY;
        idxY = idxY < 0 ? sizeY-1 : idxY;

        if(it.second.empty()){
            thisProxy(idxX,idxY).recv(nullptr,0);
        }
        else{
            double* tmp = (double*)malloc(sizeof(double)*3*it.second.size());
            for(int j=0;j<it.second.size();j++){
                tmp[3*j]= it.second[j].x;
                tmp[3*j+1] = it.second[j].y;
                tmp[3*j+2] = it.second[j].color;
            }
            thisProxy(idxX,idxY).recv(tmp,3*it.second.size());
        }
    }
    recv(nullptr,0);
    }

void ParticleGrid::recv(double* coordsRecv, int size){
    recvCount++;
    for(int i=0;i<size;i+=3){
        coords.push_back(coordinate{coordsRecv[i],coordsRecv[i+1],coordsRecv[i+2]});
    }
    if(recvCount==9){
        recvCount=0;
        numPoints = coords.size();
        CkCallback cb(CkReductionTarget(Main,getTotal),main);
        contribute(sizeof(int), &numPoints, CkReduction::sum_int, cb);
    }
}

void ParticleGrid::ResumeFromSync(){
    recvCount=0;
    numPoints = coords.size();
    CkCallback cb(CkReductionTarget(Main,getTotal),main);
    contribute(sizeof(int), &numPoints, CkReduction::sum_int, cb);
}

void ParticleGrid::getImage(liveVizRequestMsg *m){
    int pixStartX = thisIndex.x*numPix;
    int pixStartY = thisIndex.y*numPix;
    // int max = -1;
    for(int i=0;i<3*numPix*numPix;i++){
        intensity[i] = 0;
    }
    for(auto it:coords){
        int px = floor((it.x-x1)*numPix*sizeX);
        int py = floor((it.y-y1)*numPix*sizeY);
        px = px>=numPix ? numPix-1 : px;
        py = py>=numPix ? numPix-1 : py;
        px = px<0 ? 0 : px;
        py = py<0 ? 0 : py;
        if(abs(it.color-1)<1e-3){
            intensity[3*(px + numPix*py)] = 255;
            intensity[3*(px + numPix*py)+1] = 0;
            intensity[3*(px + numPix*py)+2] = 0;
        }
        else{
            intensity[3*(px + numPix*py)] = 0;
            intensity[3*(px + numPix*py)+1] = 0;
            intensity[3*(px + numPix*py)+2] = 255;
        }
    }
    liveVizDeposit(m, pixStartX,pixStartY, numPix, numPix, intensity, this);
}
