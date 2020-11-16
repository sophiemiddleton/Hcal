/**
 * @file HcalDigiProducer.h
 * @brief Class that performs basic HCal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory 
 */

#ifndef HCAL_HCALDIGIPRODUCER_H_
#define HCAL_HCALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory> //for smart pointers
#include <set> //for tracking used detector IDs

//----------//
//   LDMX   //
//----------//
#include "Event/HgcrocDigiCollection.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalDigiID.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
#include "Tools/HgcrocEmulator.h"

namespace ldmx {

/**
 * @class HcalDigiProducer
 * @brief Performs basic HCal digitization
 */
class HcalDigiProducer : public Producer {

 public:

  /**
   * Constructor
   * Makes unique noise generator and injector for this class
   */
  HcalDigiProducer(const std::string& name, Process& process);

  /**
   * Destructor
   * Deletes digi collection if it has been created
   */
  virtual ~HcalDigiProducer();

            /**
             * Configure this producer from the python configuration.
             * Sets event constants and configures the noise generator, noise injector, and pulse function.
             * Creates digi collection
             */
            virtual void configure(Parameters&);

            /**
             * Simulates measurement of pulse and creates digi collection for input event.
             */
            virtual void produce(Event& event);

        private:

            ///////////////////////////////////////////////////////////////////////////////////////
            //Python Configuration Parameters
            
            /// Time interval for chip clock in ns
            double clockCycle_;

            /// The gain in ADC units per MeV. 
            double gain_;

            /// The pedestal in ADC units 
            double pedestal_;

            /// Depth of ADC buffer. 
            int nADCs_; 

            /// Index for the Sample Of Interest in the list of digi samples 
            int iSOI_;

            /// Readout threshold [mV]
            double readoutThreshold_;

            /// Conversion from energy in MeV to voltage in mV
            double MeV_;

            ///////////////////////////////////////////////////////////////////////////////////////
            // Other member variables

            /// Put noise into empty channels, not configurable, only helpful in development
            bool noise_{true};

            /// Hgcroc Emulator to digitize analog voltage signals
            std::unique_ptr<HgcrocEmulator> hgcroc_;

            /// Conversion from time in ns to ticks of the internal clock
            double ns_;

	    // Verbose for debug purposes
	    bool verbose_{true};

    };
}

#endif
