/**
 * @file HcalClusterProducer.h
 * @brief Class that performs clustering of HCal hits
 * @author Sophie Middleton, Caltech
 */

#ifndef HCAL_HCALCLUSTERPRODUCER_H_
#define HCAL_HCALCLUSTERPRODUCER_H_

// ROOT
#include "TString.h"
#include "TRandom3.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
#include "Framework/Parameters.h" 
#include "Event/EventConstants.h"
#include "Framework/Exception.h"
#include "Framework/RandomNumberSeedService.h"

//Hcal
#include "Event/HcalHit.h"
#include "Event/HcalCluster.h"
#include "Hcal/ClusterMaker.h"
#include "DetDescr/HcalGeometry.h"
namespace ldmx {

/**
 * @class HcalClusterProducer
 * @brief Make clusters from hits in the HCAL
 */
class HcalClusterProducer : public Producer {

 public:
            
  HcalClusterProducer(const std::string& name, Process& process);

  virtual ~HcalClusterProducer() {;}

  /** 
   * Configure the processor using the given user specified parameters.
   * 
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(Parameters& parameters) final override;
            
  virtual void produce(Event& event);
  void  fillCluster(std::list<const HcalHit*>,std::vector<HcalCluster>& clusters);
 private:

  bool       verbose_{false};
  double     EminSeed_{0.};
  double     EnoiseCut_{0.};
  //double     deltaTime_{0};
 
  double     EminCluster_{0.};
  double     cutOff_{0.};
};

}

#endif
