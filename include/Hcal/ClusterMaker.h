#ifndef ClusterMaker_H_
#define ClusterMaker_H_

#include "Event/HcalHit.h"
#include "Event/HcalCluster.h"
#include "DetDescr/HcalGeometry.h"
#include <vector>
#include <queue>
#include <list>

namespace ldmx {


    class ClusterMaker 
    {
         public:
             using HitList = std::list<const HcalHit*>;
             using HitVec  = std::vector<const HcalHit*>;

             ClusterMaker(const HcalHit*, double, double);  

             void  makeCluster(std::vector<HitList>&);
             //void  fillCluster(std::lit<const HcalHit*>,std::vector<HcalCluster>& clusters);
             const HitList& clusterList() const {return clusterList_;}             
             const HcalGeometry& hcalGeoom;

         private:
            
             const HcalHit*       Seed_;
             double               seedTime_;
             HitList      clusterList_;

             double               deltaTime_; 
             double               ExpandCut_;
             
             std::queue<int>      ToVisit_;//TODO
             std::vector<bool>    isVisited_; //TODO

    };


}

#endif
