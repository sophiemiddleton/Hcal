/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic HCal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalDigiProducer.h"
#include "Framework/RandomNumberSeedService.h"

namespace ldmx {

    HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {

        //noise generator by default uses a Gausian model for noise
        //  i.e. It assumes the noise is distributed around a mean (setPedestal)
        //  with a certain RMS (setNoise) and then calculates
        //  how many hits should be generated for a given number of empty
        //  channels and a minimum readout value (setNoiseThreshold)
        //noiseGenerator_ = std::make_unique<NoiseGenerator>();
    }

    HcalDigiProducer::~HcalDigiProducer() { }

    void HcalDigiProducer::configure(Parameters& ps) {

        //settings of readout chip
        //  used  in actual digitization
        auto hgcrocParams = ps.getParameter<Parameters>("hgcroc");
        hgcroc_ = std::make_unique<HgcrocEmulator>(hgcrocParams);
        gain_             = hgcrocParams.getParameter<double>("gain");
        pedestal_         = hgcrocParams.getParameter<double>("pedestal");
        clockCycle_       = hgcrocParams.getParameter<double>("clockCycle");
        nADCs_            = hgcrocParams.getParameter<int>("nADCs");
        iSOI_             = hgcrocParams.getParameter<int>("iSOI");
	noise_            = hgcrocParams.getParameter<bool>("noise");

        // physical constants
        //  used to calculate unit conversions
        MeV_ = ps.getParameter<double>("MeV");

        //Time -> clock counts conversion
        //  time [ns] * ( 2^10 / max time in ns ) = clock counts
        ns_ = 1024./clockCycle_;

        // Configure generator that will produce noise hits in empty channels
        readoutThreshold_ = hgcrocParams.getParameter<double>("readoutThreshold");
    }

    void HcalDigiProducer::produce(Event& event) {

        if(!hgcroc_->hasSeed()) {
            const auto& rseed = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
            hgcroc_->seedGenerator(rseed.getSeed("HcalDigiProducer::HgcrocEmulator"));
        }

        //Empty collection to be filled
        HgcrocDigiCollection hcalDigis;
        hcalDigis.setNumSamplesPerDigi( nADCs_ ); 
        hcalDigis.setSampleOfInterestIndex( iSOI_ );

        std::set<unsigned int> filledDetIDs; //detector IDs that already have a hit in them

        /******************************************************************************************
         * HGCROC Emulation on Simulated Hits
         *****************************************************************************************/
        //std::cout << "Sim Hits" << std::endl;
        //get simulated hcal hits from Geant4
        //  the class HcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
        //  this class ensures that only one SimCalorimeterHit is generated per cell, but
        //  multiple "contributions" are still handled within SimCalorimeterHit 
        auto hcalSimHits{event.getCollection<SimCalorimeterHit>(EventConstants::HCAL_SIM_HITS)};

	if(verbose_)
	  std::cout << "Energy to Voltage Conversion: " << MeV_ << " mV/MeV" << std::endl;

	float strip_width(50.0f); // in mm
	double strip_attenuation_length_ = 5.; // in m
	double STRIPS_BACK_PER_LAYER_ = 60.; // n strips correspond to 5 cm wide bars  
	float half_total_width   = STRIPS_BACK_PER_LAYER_*strip_width/2.0f; // 1500 mm in one layer

        for (auto const& simHit : hcalSimHits ) {

            std::vector<double> voltages, times;
	    double sumEdep = 0; // should be the same as simHit.getEdep() 
            int sumPE = 0;
            int maxPE = 0;
	    double sumV = 0;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {
	        /* debug printout
	        std::cout << "simhit edep " << simHit.getContrib(iContrib).edep << " MeV" << std::endl;
		*/
                voltages.push_back( simHit.getContrib( iContrib ).edep*MeV_ );
                times.push_back( simHit.getContrib( iContrib ).time );
		// debug info
		sumV += simHit.getContrib( iContrib ).edep*MeV_;
		sumEdep += simHit.getContrib( iContrib ).edep;
                int PE = simHit.getContrib( iContrib ).edep*(1/4.66)*68;
                sumPE += PE;
                if(PE > maxPE)
                  maxPE = PE;
            }

            if(sumPE>0 and verbose_){
	      std::cout << "sumPE " << sumPE << " sumEdep " << sumEdep << " sumVolt " << sumV << std::endl;
	    }

            unsigned int hitID = simHit.getID();
            filledDetIDs.insert( hitID );
	    HcalID detID(hitID);
            int layer = detID.layer();
            int section = detID.section();
            int strip = detID.strip();

	    std::vector<float> position = simHit.getPosition();
	    double cur_xpos = position[0];
	    double cur_ypos = position[1];

	    // amplitude attenuation
	    float distance_along_bar = (layer % 2) ? fabs(cur_xpos) : fabs(cur_ypos);
	    double attenuation_close = exp( -1. * ((half_total_width - distance_along_bar) / 1000.) / strip_attenuation_length_ );
	    double attenuation_far   = exp( -1. * ((half_total_width + distance_along_bar) / 1000.) / strip_attenuation_length_ );

	    // time shift
	    // velocity of light in Polystyrene, n = 1.6 = c/v
	    float v = 299.792/1.6; // mm/ns
	    double shift_close = fabs((half_total_width - distance_along_bar) / v);
            double shift_far = fabs((half_total_width + distance_along_bar) / v);

            // build new digi id
            HcalDigiID closeID(section,layer,strip,0);
            HcalDigiID farID(section,layer,strip,1);
	    
	    if(sumPE>0 and verbose_){
	      std::cout << "new sim hit sumPE " << sumPE << std::endl;
	      std::cout << "hit position x " << cur_xpos  << " y " << cur_ypos << " layer " << layer  << " distance " << distance_along_bar << std::endl;
	      std::cout << "half width " << half_total_width << " close " << (half_total_width - distance_along_bar) << " att " << attenuation_close << std::endl;
	      std::cout << "shift close " << shift_close << " and v " << v << std::endl;
	      std::cout << "half width " << half_total_width << " far "<< (half_total_width + distance_along_bar) << " att " << attenuation_far << std::endl;
	      
	      std::cout << "hit ID " << hitID << " layer " << layer << " section " << section << " strip " << strip << std::endl;
	      std::cout << "digi ID close " << closeID.raw() << " layer " << layer << " section " << section << " strip " << strip << " side " << closeID.side() <<  std::endl;
	      std::cout << "digi ID far " << farID.raw() <<" layer " << layer << " section " << section << " strip " << strip << " side " << farID.side() <<  std::endl;
	      std::cout << hitID << " " << simHit.getEdep() << std::endl;
	    }

            //container emulator uses to write out samples and  
	    // transfer samples into the digi collection
	    std::vector<HgcrocDigiCollection::Sample> digiToAdd;

	    // digitize
            if ( hgcroc_->digitize( hitID , voltages , times , digiToAdd, attenuation_close, shift_close) ) {
	      hcalDigis.addDigi( closeID.raw() , digiToAdd, sumEdep, sumPE, maxPE, strip, layer );
            }

	    if(sumPE>0 and verbose_)
	      std::cout << " now digitize far " << std::endl;
	    if ( hgcroc_->digitize( hitID , voltages , times , digiToAdd, attenuation_far, shift_far) ) {
              hcalDigis.addDigi( farID.raw() , digiToAdd, sumEdep, sumPE, maxPE, strip, layer );
            }
        }

        event.add("HcalDigis", hcalDigis );

        return;
    } //produce

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);
