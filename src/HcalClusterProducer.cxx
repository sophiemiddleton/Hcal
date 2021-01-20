#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "Hcal/HcalClusterProducer.h"
#include "Framework/Exception.h"
#include "Framework/RandomNumberSeedService.h"

#include <iostream>
#include <exception>

namespace ldmx {

HcalClusterProducer::HcalClusterProducer(const std::string& name, Process& process) :
    Producer(name, process){}

void HcalClusterProducer::configure(Parameters& parameters) {
     EminSeed_         = parameters.getParameter< std::double >("minimum energy of seed");
}


void HcalClusterProducer::produce(Event& event)
{
    std::vector<HcalCluster> hcalClusters;
    std::vector<HcalHit> seedList;
     // looper over sim hits and find clusers:
    auto hcalHits{ event.getCollection<HcalHit>()};
    for (const HcalHit hit : hcalHits ) {
        seedList.push_back(hit);
    }
    seedList.sort([](const HcalHit* a, const HcalHit* b) {return a->getEnergy() > b->getEnergy();});
    //Seed = Event()
    double most_energetic = 0;
    int seed_id = -1;
    std::vector<Hcalhit> hits;
    Hcalhit seed;

    while(!seedList.empty() ){
        const HcalHit* Seed = *seedList.begin();
        if (Seed->getEnergy() < EminSeed_) break;

        finder(Seed);
        makeCluster(caloIdHitMap);//TODO

        clusterList.push_back(finder.clusterList());
        for (const auto& hit: finder.clusterList()) seedList.remove(hit);

    }
    for (const HcalHit hit : hcalHits ) {
        if (hit.isNoise() == 0){
            hits.push_back(hit)
            if(hit.getPE() > most_energetic){
                most_energetic = hit.getPE()
                seed = hit
                seed_id = ih
            }
        }
    }
    event.add( "HcalClusters", hcalClusters );
}

DECLARE_PRODUCER_NS(ldmx, HcalClusterProducer);

