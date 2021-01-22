#ifndef ClusterMaker_H_
#define ClusterMaker_H_

#include "Event/HcalHit.h"

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
             const HitList& clusterList() const {return clusterList_;}             


         private:
            
             const HcalHit*       Seed_;
             double               seedTime_;
             HitList      clusterList_;
             std::queue<int>      ToVisit_;//TODO
             std::vector<bool>    isVisited_; //TODO
             double               deltaTime_; 
             double               ExpandCut_;

    };


}

#endif
