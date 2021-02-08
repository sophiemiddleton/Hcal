#ifndef ClusterMaker_H_
#define ClusterMaker_H_

#include "Event/HcalHit.h"
#include "Event/HcalCluster.h"
#include "DetDescr/HcalGeometry.h"
#include <vector>
#include <queue>
#include <list>
#include "TLorentzVector.h"
#include "Event/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventProcessor.h"
namespace ldmx {
   
    //template <class WeightClass>
    class ClusterMaker 
    {
         public:
            using HitList = std::list<const HcalHit*>;
            using HitVec  = std::vector<const HcalHit*>;
            using ClusterList = std::list<const HcalCluster*>;
            explicit ClusterMaker(const HcalGeometry& hcalGeom_) : hcalGeom(hcalGeom_){};
            //ClusterMaker(const HcalHit*, double, double);  
            std::vector<HcalCluster> getClusters();
            //void makeCluster(std::vector<HitList>&);
            void makeClusterv2(double seed_threshold, double cutoff);
            void addHits(const HcalHit* Seed);
            void addCluster(const HcalCluster cluster);
          
            const HitList& hitList() const {return hitList_;}             
            
            HcalCluster cluster;

            HitVec hits_;
            TLorentzVector centroid_;
            const HcalGeometry& hcalGeom;

            std::vector<HcalCluster> working_;//TODO list?
            std::map<int, double> transitionWeights_;
            HcalCluster wf;
            double finalwgt_;
            int nseeds_;
            //WeightClass wgt_;
         private:
            
             const HcalHit*       Seed_;
             double               seedTime_;
             HitList              hitList_;
             double               deltaTime_; 
          
    };


}

#endif
