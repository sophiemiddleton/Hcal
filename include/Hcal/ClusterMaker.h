#ifndef ClusterMaker_HH_
#define ClusterMaker_HH_

#include "Event/HcalHit.h"

#include <vector>
#include <queue>
#include <list>

namespace mu2e {


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
             std::queue<int>      ToVisit_;
             std::vector<bool>    isVisited_; 
             double               deltaTime_; 

    };


}

#endif
