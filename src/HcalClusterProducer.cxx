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
         EnoiseCut_ = parameters.getParameter< double >("EnoiseCut");
       //  deltaTime_ = parameters.getParameter< double >("deltaTime");

         EminCluster_ = parameters.getParameter< double >("EminCluster");
         cutOff_ = parameters.getParameter< double >("cutOff");
        
    }


    void HcalClusterProducer::produce(Event& event)
    {
        std::cout<<"[HcalClusterProducer::produce beginning...]"<<std::endl;
        std::vector<HcalCluster> hcalClusters;
        std::list<const HcalHit*> seedList;
        std::vector<std::list<const HcalHit*>> clusterList, hitMap;
        std::vector< HcalHit > hcalHits = event.getCollection< HcalHit >("HcalRecHits");
        

        if (hcalHits.empty()) { return; }
        
        for (auto const hit : hcalHits ) {
            if (hit.getEnergy() <  EnoiseCut_) continue;
            seedList.push_back(&hit);
        }
        //Sort the list of seed hits:
        
        seedList.sort([](const HcalHit* a, const HcalHit* b) {return a->getEnergy() > b->getEnergy();});
        const HcalGeometry& hcalGeom = getCondition<ldmx::HcalGeometry>(ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
        ClusterMaker finder(hcalGeom);
       
        //Loop over hits in seed list, form "protoclusters":
        for (const HcalHit* Seed : seedList ) { //while(!seedList.empty() ){
            //const HcalHit* Seed = *seedList.begin();
            if (Seed->getEnergy() < EminSeed_) break;
            
            //Add Seed to the cluster - builds a single cluster with hits, need to find split clusters
            finder.addHits(Seed);
         
            //Add list of hits in cluster to the clusterList:
            //clusterList.push_back(finder.hitList());//TODO - is this need?
            //Remove seeds which have been included in this cluster - currently this is all hits...
            //for (const auto& hit: finder.hitList()) seedList.remove(hit);

        }
        finder.makeCluster(EminCluster_,cutOff_);
        std::cout<<"[HcalClusterProducer::produce cluster made...]"<<std::endl;
        std::vector<HcalCluster> wcVec = finder.getClusters();
        std::cout<<"[HcalClusterProducer::produce ending...]"<<std::endl;
        for (unsigned int c = 0; c < wcVec.size(); c++) {
    
            HcalCluster cluster;
    
            cluster.setEnergy(wcVec[c].centroid().E());
            cluster.setCentroidXYZ(wcVec[c].centroid().Px(), wcVec[c].centroid().Py(), wcVec[c].centroid().Pz());
            cluster.setNHits(wcVec[c].getHits().size());
            cluster.addHits(wcVec[c].getHits());
            std::cout<<"[HcalClusterProducer::setting the cluster parameters...]"<<std::endl;
            hcalClusters.push_back( cluster );
        }
        std::cout<<"[HcalClusterProducer::produce adding to event...]"<<std::endl;
        event.add( "HcalClusters", hcalClusters );
    }
    ////for(auto const& cluster_list : hitList) fillCluster(cluster_list, hcalClusters);
    /*void  HcalClusterProducer::fillCluster(std::list<const HcalHit*> cluster_list,std::vector<HcalCluster>& hcalClusters){
        std::vector<HcalHit> caloHitsPtrVector;

        double totalEnergy(0), xcl(0), ycl(0), zcl(0);
        for(auto const& hit : cluster_list){
                
                totalEnergy    += hit->getEnergy();
                //double x,y,z;
                //hcalGeometry.getStripAbsolutePosition( id , x , y , z );
                ///xcl += x*clusterPrt->energyDep();
                //ycl += y*clusterPrt->energyDep();
                //zcl += z*clusterPrt->energyDep();
                //caloHitsPtrVector.push_back( art::Ptr<CaloHit>(CaloHitsHandle,idx) );
                
            }
        HcalCluster cluster;
        cluster.setEnergy(totalEnergy );
        cluster.setCentroidXYZ(xcl,ycl,zcl);
        //cluster->setNHits(cluster->getHits().size());
        //cluster->addHits(cluster->getHits());
        hcalClusters.push_back( cluster );
    }*/
}
DECLARE_PRODUCER_NS(ldmx, HcalClusterProducer);

