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
 private:

  bool                            verbose_{false};
 
};

}

#endif
