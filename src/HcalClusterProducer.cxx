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
    Producer(name, process)
{}

void HcalClusterProducer::configure(Parameters& parameters) {}


void HcalClusterProducer::produce(Event& event)
{}

  
//  event.add( "HcalRecHits", hcalRecHits );

DECLARE_PRODUCER_NS(ldmx, HcalClusterProducer);

