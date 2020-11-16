/**
 * @file HcalRecProducer.h
 * @brief Class that performs basic HCal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#ifndef HCAL_HCALRECPRODUCER_H_
#define HCAL_HCALRECPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory> //for smart pointers

//----------//
//   LDMX   //
//----------//
#include "Event/EventDef.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalDigiID.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class HcalRecProducer
     * @brief Performs basic HCal reconstruction
     *
     * Reconstruction is done from the HcalDigi samples.
     * Some hard-coded parameters are used for position and energy calculation.
     */
    class HcalRecProducer : public Producer {

        public:

            /**
             * Constructor
             */
            HcalRecProducer(const std::string& name, Process& process);

            /**
             * Destructor
             */
            virtual ~HcalRecProducer();

            /**
             * Grabs configure parameters from the python config file.
             */
            virtual void configure(Parameters&);

            /**
             * Produce HcalHits and put them into the event bus using the
             * HcalDigis as input.
             *
             * This function unfolds the digi samples taken by the HGC ROC
             * and reconstructs their energy using knowledge of how
             * the chip operates and the position using HcalHexReadout.
             */
            virtual void produce(Event& event);

        private:
            /** Digi Collection Name to use as input */
            std::string digiCollName_;

            /** Digi Pass Name to use as input */
            std::string digiPassName_;

	    /// simhit collection name
	    std::string simHitCollName_;

            /// simhit pass name
	    std::string simHitPassName_;

            /// output hit collection name
	    std::string recHitCollName_;

            /// Energy [MeV] deposited by a MIP in Si 0.5mm thick
            double mip_si_energy_;

            /// Length of clock cycle [ns]
            double clock_cycle_;

	    /// Voltage by average MIP 
            double voltage_per_mip_;

    };
}

#endif
