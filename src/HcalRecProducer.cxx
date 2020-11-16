/**
 * @file HcalRecProducer.cxx
 * @brief Class that performs basic HCal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalRecProducer.h"
#include "Hcal/HcalReconConditions.h"
#include "DetDescr/HcalReadout.h"

namespace ldmx {

    HcalRecProducer::HcalRecProducer(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    HcalRecProducer::~HcalRecProducer() { }

    void HcalRecProducer::configure(Parameters& ps) {

        digiCollName_ = ps.getParameter<std::string>( "digiCollName" );
        digiPassName_ = ps.getParameter<std::string>( "digiPassName" );
        simHitCollName_ = ps.getParameter<std::string>("simHitCollName");
        simHitPassName_ = ps.getParameter<std::string>("simHitPassName");
        recHitCollName_ = ps.getParameter<std::string>("recHitCollName");

	mip_si_energy_  = ps.getParameter<double>( "mip_si_energy"  );
        clock_cycle_    = ps.getParameter<double>( "clock_cycle"    );
        voltage_per_mip_ = ps.getParameter<double>( "voltage_per_mip" );
    }

    void HcalRecProducer::produce(Event& event) {
        // Get the Hcal Geometry
        const HcalReadout& hcalReadout = getCondition<HcalReadout>(HcalReadout::CONDITIONS_OBJECT_NAME);

        std::vector <HcalHit> hcalRecHits;
	// Get the reconstruction parameters
        HcalReconConditions the_conditions(getCondition<DoubleTableCondition>(HcalReconConditions::CONDITIONS_NAME));

        auto hcalDigis = event.getObject<HgcrocDigiCollection>( digiCollName_ , digiPassName_ );
        int numDigiHits = hcalDigis.getNumDigis();
        //loop through digis
        for ( unsigned int iDigi = 0; iDigi < numDigiHits-1; iDigi+=2 ) {
            
            auto digi_close = hcalDigis.getDigi( iDigi );
	    auto digi_far = hcalDigis.getDigi( iDigi+1 );

            //ID from first digi sample
            //  assuming rest of samples have same ID
            HcalDigiID id_close(digi_close.id());
	    HcalDigiID id_far(digi_far.id());
	    HcalID id(id_close.section(),id_close.layer(),id_close.strip());
            
            //TOA is the time of arrival with respect to the 25ns clock window
            //  TODO what to do if hit NOT in first clock cycle?
            double timeRelClock25_close = digi_close.begin()->toa()*(clock_cycle_/1024); //ns
	    double timeRelClock25_far   = digi_far.begin()->toa()*(clock_cycle_/1024); //ns
            double hitTime = fabs(timeRelClock25_close-timeRelClock25_far)/2;
	    std::cout << " got time " << std::endl;

	    //ID to real space position
            double x,y,z;
	    hcalReadout.getStripAbsolutePosition( id , x , y , z );

	    //get the estimated voltage deposited from digi samples
            double voltage(0.);

	    //ADC mode of readout
	    TH1F measurements_close( "measurements_close" , "measurements_close" ,
				     10.*clock_cycle_ , 0. , 10.*clock_cycle_ );
	    TH1F measurements_far( "measurements_far" , "measurements_far" ,
				   10.*clock_cycle_ , 0. , 10.*clock_cycle_ );
	    
	    double maxMeas_close{0.};
	    int numWholeClocks{0};
	    for ( auto it = digi_close.begin(); it < digi_close.end(); it++) {
	      double amplitude     = (it->adc_t() - the_conditions.adcPedestal(id_close))*the_conditions.adcGain(id_close);
	      double time          = numWholeClocks*clock_cycle_; //+ offestWithinClock; //ns
	      measurements_close.Fill( time , amplitude );
	      if ( amplitude > maxMeas_close ) maxMeas_close = amplitude;
	    }
	    
	    double maxMeas_far{0.};
	    numWholeClocks = 0;
	    for ( auto it = digi_far.begin(); it < digi_far.end(); it++) {
	      double amplitude     = (it->adc_t() - the_conditions.adcPedestal(id_far))*the_conditions.adcGain(id_far);
	      double time          = numWholeClocks*clock_cycle_; //+ offestWithinClock; //ns                                                                                  
	      measurements_far.Fill( time , amplitude );
	      if ( amplitude > maxMeas_far ) maxMeas_far = amplitude;
	    }
	    
	    //just use the maximum measured voltage for now
	    //std::cout << "maxmeas close " << maxMeas_close << " maxmeas_far " << maxMeas_far << " mV " << mV_ << std::endl;
	    voltage = maxMeas_close + maxMeas_far; // mV
            
	    double num_mips_equivalent = voltage / voltage_per_mip_;
            double energy_deposited_in_Si = num_mips_equivalent * mip_si_energy_;

	    /* debug printout
	       std::cout << " -> " << num_mips_equivalent
	       << " equiv MIPs -> " << energy_deposited_in_Si << " MeV"
	       << std::endl;
	    */

	    // TODO need to incorporate layer weights and second Order correction
	    double reconstructed_energy = energy_deposited_in_Si;

            //copy over information to rec hit structure in new collection
            HcalHit recHit;
            recHit.setID( id.raw() );
            recHit.setXPos( x );
            recHit.setYPos( y );
            recHit.setZPos( z );
	    recHit.setAmplitude( energy_deposited_in_Si );
            recHit.setEnergy( reconstructed_energy );
            recHit.setTime( hitTime );

            hcalRecHits.push_back( recHit );
        }

        if (event.exists("HcalSimHits")) {
	    //hcal sim hits exist ==> label which hits are real and which are pure noise
	    auto hcalSimHits{event.getCollection<SimCalorimeterHit>("HcalSimHits")};
            std::set<int> real_hits;
            for ( auto const& sim_hit : hcalSimHits ) real_hits.insert( sim_hit.getID() );
            for ( auto& hit : hcalRecHits ) hit.setNoise( real_hits.find(hit.getID()) == real_hits.end() );
        }

        //add collection to event bus
        event.add( "HcalRecHits", hcalRecHits );
    }

}

DECLARE_PRODUCER_NS(ldmx, HcalRecProducer);
