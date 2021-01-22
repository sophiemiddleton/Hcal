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
        std::list<const HcalHit*> seedList, hitMap;
        std::vector< HcalHit > hcalHits = event.getCollection< HcalHit >("HcalHits");
        

        if (hcalHits.empty()) { return; }
        
        for (auto const hit : hcalHits ) {
            if (hit.getEnergy() <  EnoiseCut_) continue;
            seedList.push_back(&hit);
        }
        // Sort hits by energy:
        seedList.sort([](const HcalHit* a, const HcalHit* b) {return a->getEnergy() > b->getEnergy();});
      
        double most_energetic = 0;
        int seed_id = -1;
        HcalHit seed;
       
        while(!seedList.empty() ){
            const HcalHit* Seed = *seedList.begin();
            if (Seed->getEnergy() < EminSeed_) break;
            ClusterMaker finder(Seed,deltaTime_,expandCut_);
           
            finder.makeCluster(hitMap);

            clusterList.push_back(finder.clusterList());
            for (const auto& hit: finder.clusterList()) seedList.remove(hit);

        }
        
        event.add( "HcalClusters", hcalClusters );
    }
}
DECLARE_PRODUCER_NS(ldmx, HcalClusterProducer);

