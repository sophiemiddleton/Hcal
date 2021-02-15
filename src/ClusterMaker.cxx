#include "Hcal/ClusterMaker.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace ldmx {


    std::vector<HcalCluster>  ClusterMaker::getClusters(){
        std::cout<<"List of working cluster is of size : "<<working_.size()<<std::endl;
        return working_;
    }
    
    static bool compClusters(const HcalCluster& a, const HcalCluster& b) {
                return a.getCentroidE() > b.getCentroidE();
     }
            
    void ClusterMaker::addHits(const HcalHit* Seed){
        
        double hitE = Seed->getEnergy();

        double hitX, hitY, hitZ;
        hcalGeom.getStripAbsolutePosition( Seed->getID(), hitX , hitY , hitZ );

        double newE = hitE + wf.getCentroidE();
        double newCentroidX = (wf.getCentroidX()*wf.getCentroidE() + hitE*hitX) / newE;
        double newCentroidY = (wf.getCentroidY()*wf.getCentroidE() + hitE*hitY) / newE;
        double newCentroidZ = (wf.getCentroidZ()*wf.getCentroidE() + hitE*hitZ) / newE;

        wf.setCentroidXYZE(newCentroidX, newCentroidY, newCentroidZ, newE);
        std::cout<<"New centre: "<<newCentroidX<<" "<<newCentroidY<<" "<<newCentroidZ<<" "<<newE<<std::endl;
        wf.addHittoCluster(Seed); 
        working_.push_back(wf);
    }
    
    void ClusterMaker::addCluster(const HcalCluster cluster) {
    
        double clusterE  = cluster.getCentroidE() ;
        double centroidX = cluster.getCentroidX() ;
        double centroidY = cluster.getCentroidY() ;
        double centroidZ = cluster.getCentroidZ() ;
    
        double newE = clusterE + wf.getCentroidE();
        double newCentroidX = (wf.getCentroidX()*wf.getCentroidE() + clusterE*centroidX) / newE;
        double newCentroidY = (wf.getCentroidY()*wf.getCentroidE() + clusterE*centroidY) / newE;
        double newCentroidZ = (wf.getCentroidZ()*wf.getCentroidE() + clusterE*centroidY) / newE;
    
        wf.setCentroidXYZE(newCentroidX, newCentroidY, newCentroidZ, newE);

        std::vector<const HcalHit*> clusterHits = cluster.getHits();
    
        for (unsigned int i = 0; i < clusterHits.size(); i++) {
    
            wf.addHittoCluster(clusterHits[i]);
        }
        
    }
    void ClusterMaker::add(HcalCluster &oldcluster, const HcalCluster cluster) {
    
        double clusterE  = cluster.getCentroidE() ;
        double centroidX = cluster.getCentroidX() ;
        double centroidY = cluster.getCentroidY() ;
        double centroidZ = cluster.getCentroidZ() ;
    
        double newE = clusterE + oldcluster.getCentroidE();
        double newCentroidX = (oldcluster.getCentroidX()*oldcluster.getCentroidE() + clusterE*centroidX) / newE;
        double newCentroidY = (oldcluster.getCentroidY()*oldcluster.getCentroidE() + clusterE*centroidY) / newE;
        double newCentroidZ = (oldcluster.getCentroidZ()*oldcluster.getCentroidE() + clusterE*centroidY) / newE;
    
        oldcluster.setCentroidXYZE(newCentroidX, newCentroidY, newCentroidZ, newE);

        std::vector<const HcalHit*> clusterHits = cluster.getHits();
    
        for (unsigned int i = 0; i < clusterHits.size(); i++) {
    
            oldcluster.addHittoCluster(clusterHits[i]);
        }
        
    }
    

    
    void ClusterMaker::makeCluster(double seed_threshold, double cutoff) {
                std::cout<<"[makeCluster: starting...]"<<std::endl;
                unsigned int ncluster = working_.size();
                double minwgt = cutoff;
                //Sort list of working clusters:
                std::cout<<"[makeCluster: sorting...]"<<std::endl;
                std::sort(working_.begin(), working_.end(), compClusters);
                do {
                    bool any = false;
                    size_t mi(0),mj(0);
                    std::cout<<"[makeCluster: doing...]"<<std::endl;
                    int nseeds = 0;
                    // Loop over clusters
                    for (unsigned int i = 0; i < working_.size(); i++) {
                        std::cout<<"[makeCluster: sinside working loop...]"<<std::endl;
                        if (working_[i].empty()) continue;

                        bool passes_threshold = (working_[i].centroid().E() >= seed_threshold);
                        if (passes_threshold) {
                            nseeds++;
                        } else {
                            break;
                        }
                        std::cout<<"[makeCluster: passes threshold...]"<<std::endl;
                        for (unsigned int j = i + 1; j < working_.size(); j++) {
    
                            if (working_[j].empty() or (!passes_threshold and working_[j].getCentroidE() < seed_threshold)) continue;
                            double wgt = working_[i].getEnergy();//wgt_(working_.clusters_[i],working_.clusters_[j]); //TODO - what does this do?
                            if (!any or wgt < minwgt) {
                                any = true;
                                minwgt = wgt;
                                mi = i;
                                mj = j;
                            }
                        }
                    }
                    std::cout<<"[makeCluster: ending...]"<<std::endl;
                    nseeds_ = nseeds;
                    transitionWeights_.insert(std::pair<int, double>(ncluster, minwgt)); 
                    // combine clusters
                    if (any and minwgt < cutoff) {
                        // put the bigger one in mi
                        if (working_[mi].getCentroidE() < working_[mj].getCentroidE()) { std::swap(mi,mj); }
                        // now we have the smallest, merge
                        this->add(working_[mi],working_[mj]);
                        working_[mj].clear();
                        // decrement cluster count
                        ncluster--;
                    } 
        
                } while (minwgt < cutoff and ncluster > 1);
                std::cout<<"[makeCluster: updating final wgt...]"<<std::endl;
                finalwgt_ = minwgt;
            } 
    //void ClusterMaker::makeCluster(std::vector<HitList>& idHitVec)  
    //{ 
     ////   hitList_.clear();            
     //   hitList_.push_front(Seed_);

        //crystalToVisit_.push(Seed_->crystalID());  

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
                    hitList_.push_front(hit);
                    it = list.erase(it);   
                } 
                else ++it;
            } 
        }

        ToVisit_.pop();                 
        }*/
        //hitList_.sort([](const HcalHit* lhs, const HcalHit* rhs) {return lhs->getEnergy() > rhs->getEnergy();});
       //} 
       
      
}
