#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "Hcal/HcalClusterProducer.h"

#include <iostream>
#include <exception>

namespace ldmx {

    HcalClusterProducer::HcalClusterProducer(const std::string& name, Process& process) :
        Producer(name, process){}

    void HcalClusterProducer::configure(Parameters& parameters) {
         EminSeed_ = parameters.getParameter< double >("EminSeed");
         EnoiseCut_ = parameters.getParameter< double >("noiseCut");
         deltaTime_ = parameters.getParameter< double >("deltaTime");
         expandCut_ = parameters.getParameter< double >("expandCut");
    }


    void HcalClusterProducer::produce(Event& event)
    {
        std::vector<HcalCluster> hcalClusters;
        std::list<const HcalHit*> seedList;
        std::vector<std::list<const HcalHit*>> clusterList, hitMap;
        std::vector< HcalHit > hcalHits = event.getCollection< HcalHit >("HcalHits");
        

        if (hcalHits.empty()) { return; }
        
        for (auto const hit : hcalHits ) {
            if (hit.getEnergy() <  EnoiseCut_) continue;
            seedList.push_back(&hit);
        }

        seedList.sort([](const HcalHit* a, const HcalHit* b) {return a->getEnergy() > b->getEnergy();});
      
        while(!seedList.empty() ){
            const HcalHit* Seed = *seedList.begin();
            if (Seed->getEnergy() < EminSeed_) break;
            ClusterMaker finder(Seed,deltaTime_,expandCut_);
           
            finder.makeCluster(hitMap);

            clusterList.push_back(finder.clusterList());
            for (const auto& hit: finder.clusterList()) seedList.remove(hit);

        }
        for(auto const& cluster_list : clusterList) fillCluster(cluster_list, hcalClusters);
        
        event.add( "HcalClusters", hcalClusters );
    }
    void  HcalClusterProducer::fillCluster(std::list<const HcalHit*> cluster_list,std::vector<HcalCluster>& hcalClusters){
        std::vector<HcalHit> caloHitsPtrVector;

        double totalEnergy(0), xcl(0), ycl(0), zcl(0);
        for(auto const& hit : cluster_list){
                
                totalEnergy    += hit->getEnergy();
                //double x,y,z;
                //hcalGeometry.getStripAbsolutePosition( id , x , y , z );
                /*xcl += x*clusterPrt->energyDep();
                ycl += y*clusterPrt->energyDep();
                zcl += z*clusterPrt->energyDep();
                //caloHitsPtrVector.push_back( art::Ptr<CaloHit>(CaloHitsHandle,idx) );*/
                
            }
        HcalCluster cluster;
        cluster.setEnergy(totalEnergy );
        cluster.setCentroidXYZ(xcl,ycl,zcl);
        /*cluster->setNHits(cluster->getHits().size());
        cluster->addHits(cluster->getHits());*/
        hcalClusters.push_back( cluster );
    }
}
DECLARE_PRODUCER_NS(ldmx, HcalClusterProducer);

