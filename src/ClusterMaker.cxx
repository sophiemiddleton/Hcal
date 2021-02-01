#include "Hcal/ClusterMaker.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace ldmx {

    ClusterMaker::ClusterMaker(const HcalHit* Seed, double deltaTime, double ExpandCut) : Seed_(Seed), seedTime_(Seed->getTime()),deltaTime_(deltaTime), ExpandCut_(ExpandCut), ToVisit_(), isVisited_(0) 
    {}

    void ClusterMaker::makeCluster(std::vector<HitList>& idHitVec)  
    { 
        clusterList_.clear();            
        clusterList_.push_front(Seed_);

        // crystalToVisit_.push(Seed_->crystalID());  

        /*HitList& liste = idHitVec[crystalSeed_->crystalID()];//TODO
        liste.erase(std::find(liste.begin(), liste.end(), Seed_));

        while (!ToVisit_.empty())
        {            
        int visitId         = ToVisit_.front();
        isVisited_[visitId] = 1;

        neighborsId.insert(neighborsId.end(), cal_->nextNeighbors(visitId).begin(), cal_->nextNeighbors(visitId).end()); //TODO

        for (auto& iId : neighborsId)
        {               
            if (isVisited_[iId]) continue;
            isVisited_[iId]=1;
            CaloCrystalList& list = idHitVec[iId];

            auto it=list.begin();
            while(it != list.end())
            {
                CaloHit const* hit = *it;
                if (std::abs(hit->time() - seedTime_) < deltaTime_)
                { 
                    if (hit->energyDep() > ExpandCut_) crystalToVisit_.push(iId);
                    clusterList_.push_front(hit);
                    it = list.erase(it);   
                } 
                else ++it;
            } 
        }

        ToVisit_.pop();                 
        }*/
        clusterList_.sort([](const HcalHit* lhs, const HcalHit* rhs) {return lhs->getEnergy() > rhs->getEnergy();});
       } 
       
      
}
