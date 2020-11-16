/**
 * @file HcalDigiPipelineTest.cxx
 * @brief Test to make sure input sim energies are close to output rec energies
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/EventProcessor.h"
#include "Framework/Process.h" 
#include "Framework/ConfigurePython.h"
#include "Event/EventDef.h" //Need event bus passengers
#include "DetDescr/HcalID.h" //creating unique cell IDs

#include <fstream> //to write config file

namespace ldmx {
namespace test {
namespace hcal {

/**
 * Energy deposited by one MIP on average.
 * [MeV]
 */
static const double MIP_ENERGY=4.66;

/**
 * Conversion between voltage and deposited energy
 * [MeV/mV]
 */
static const double MeV_per_mV=MIP_ENERGY/(37*0.162);
  PE*(5/1)
/**
 * Maximum absolute error that a single hit
 * can be reconstructed with before failing the test
 * if below the adc threshold
 *
 * Comparing energy deposited in Silicon that was
 * "simulated" (input into digitizer) and the reconstructed
 * energy deposited output by reconstructor.
 */
static const double MAX_ENERGY_ERROR_DAQ_ADC_MODE=MIP_ENERGY/2;

/**
 * Number of sim hits to create.
 *
 * In this test, we create one sim hit per event,
 * run it through the digi pipeline, and then
 * check it. This parameter tells us how many
 * sim hits to create and then (combined with
 * the parameters of HcalFakeSimHits), we know
 * how "fine-grained" the test is.
 */
static const int NUM_TEST_SIM_HITS=1;

/**
 * Should the sim/rec/tp energies be ntuplized
 * for your viewing?
 */
static const bool NTUPLIZE_ENERGIES=true;

/**
 * @class FakeSimHits
 *
 * Fills the event bus with an HcalSimHits collection with
 * a range of energy hits. These hits are put into unique
 * cells so that we can compare them to the correct energy
 * in one event.
 */
class HcalFakeSimHits : public Producer {

        /**
         * Maximum energy to make a simulated hit for [MeV]
         *
         * The maximum value to be readout is 4096 TDC which
         * is equivalent to ~10000fC deposited charge.
         */
        const double maxEnergy_  = 10000.*MeV_per_mV;

        /**
         * Minimum energy to make a sim hit for [MeV]
         * Needs to be above readout threshold (after internal HcalDigi's calculation)
         *
         * One MIP is ~0.13 MeV, so we choose that.
         */
        const double minEnergy_ = MIP_ENERGY;

        /**
         * The step between energies is calculated depending on the min, max energy
         * and the total number of sim hits you desire.
         * [MeV]
         */
        const double energyStep_ = (maxEnergy_-minEnergy_)/NUM_TEST_SIM_HITS;

        /// current energy of the sim hit we are on
        double currEnergy_ = minEnergy_;

    public:

        HcalFakeSimHits(const std::string &name,Process& p) : Producer( name , p ) { }
        ~HcalFakeSimHits() { }

        void beforeNewRun(RunHeader& header) {
            header.setDetectorName("ldmx-det-v12");
        }

        void produce(Event& event) final override {

            //put in a single sim hit
            std::vector<SimCalorimeterHit> pretendSimHits(1);

	    //since this is 000 we won't test positions
            HcalID id(0,0,0);
            pretendSimHits[0].setID( id.raw() );
            pretendSimHits[0].addContrib(
					 -1 //incidentID
					 , -1 // trackID
					 , 0 // pdg ID
					 , currEnergy_ // edep
					 , 1. //time
					 );

            //needs to be correct collection name
            REQUIRE_NOTHROW(event.add( "HcalSimHits" , pretendSimHits ));

            currEnergy_ += energyStep_;
            
            return;
        }
}; //HcalFakeSimHits

/**
 * @class HcalCheckEnergyReconstruction
 *
 * Checks
 * - Amplitude of HcalRecHit matches SimCalorimeterHit EDep with the same ID
 * - Estimated energy at TP level matches sim energy
 *
 * Assumptions
 * - Only one sim hit per event
 * - Noise generation has been turned off
 */
class HcalCheckEnergyReconstruction : public Analyzer {

    public:

        HcalCheckEnergyReconstruction(const std::string& name,Process& p) : Analyzer( name , p ) { }
        ~HcalCheckEnergyReconstruction() { }

        void onProcessStart() final override {

            getHistoDirectory();
            ntuple_.create("HcalDigiTest");
            ntuple_.addVar<float>("HcalDigiTest","SimEnergy");
            ntuple_.addVar<float>("HcalDigiTest","RecEnergy");

            ntuple_.addVar<int>("HcalDigiTest","DaqDigi");
            ntuple_.addVar<int>("HcalDigiTest","DaqDigiIsADC");
            ntuple_.addVar<int>("HcalDigiTest","DaqDigiADC");
            ntuple_.addVar<int>("HcalDigiTest","DaqDigiTOT");

        }

        void analyze(const Event& event) final override {

            const auto simHits = event.getCollection<SimCalorimeterHit>( "HcalSimHits" );

            REQUIRE( simHits.size() == 1 );

            float truth_energy = simHits.at(0).getEdep();
            ntuple_.setVar<float>("SimEnergy",truth_energy);

            const auto daqDigis{event.getObject<HgcrocDigiCollection>("HcalDigis")};

            CHECK( daqDigis.getNumDigis() == 2 ); // 2 digi objects for now for all sections
            auto daqDigi = daqDigis.getDigi(0);
            ntuple_.setVar<int>("DaqDigi",daqDigi.soi().raw());
            bool is_in_adc_mode = daqDigi.isADC();
            ntuple_.setVar<int>("DaqDigiIsADC",is_in_adc_mode);
            ntuple_.setVar<int>("DaqDigiADC",daqDigi.soi().adc_t());
            ntuple_.setVar<int>("DaqDigiTOT",daqDigi.tot());

            const auto recHits = event.getCollection<HcalHit>( "HcalRecHits" );
            CHECK( recHits.size() == 1 );

            auto hit = recHits.at(0);
            HcalID id(hit.getID());
            CHECK_FALSE( hit.isNoise() );
            CHECK( id.raw() == simHits.at(0).getID() );

            //define target energy by using the settings at the top
            auto target_daq_energy = Approx(truth_energy).margin(MAX_ENERGY_ERROR_DAQ_ADC_MODE);
            if (is_in_adc_mode) target_daq_energy = Approx(truth_energy).margin(MAX_ENERGY_ERROR_DAQ_ADC_MODE);

            CHECK( hit.getAmplitude() == target_daq_energy );
            ntuple_.setVar<float>("RecEnergy",hit.getAmplitude());

            return;
        }
};//HcalCheckEnergyReconstruction

} //hcal
} //test
} //ldmx

DECLARE_PRODUCER_NS(ldmx::test::hcal,HcalFakeSimHits)
DECLARE_ANALYZER_NS(ldmx::test::hcal,HcalCheckEnergyReconstruction)

/**
 * Test for the Hcal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *
 * Checks
 *  - Keep reconstructed energy depositied close to simulated value
 *  - Keep estimated energy at TP level close to simulated value
 *
 * @TODO still need to expand to multiple contribs in a single sim hit
 * @TODO check layer weights are being calculated correctly somehow
 */
TEST_CASE( "Hcal Digi Pipeline test" , "[Hcal][functionality]" ) {

    const std::string config_file{"/tmp/hcal_digi_pipeline_test.py"};
    std::ofstream cf( config_file );

    cf << "from LDMX.Framework import ldmxcfg" << std::endl;
    cf << "p = ldmxcfg.Process( 'testHcalDigis' )" << std::endl;
    cf << "p.maxEvents = " << ldmx::test::hcal::NUM_TEST_SIM_HITS << std::endl;
    cf << "from LDMX.Hcal import hcal_hardcoded_conditions" << std::endl;
    cf << "from LDMX.Hcal import digi" << std::endl;
    cf << "p.outputFiles = [ '/tmp/hcal_digi_pipeline_test.root' ]" << std::endl;
    cf << "p.histogramFile = '" << (ldmx::test::hcal::NTUPLIZE_ENERGIES ? "" : "/tmp/") << "hcal_digi_pipeline_test.root'" << std::endl;
    cf << "from LDMX.Hcal import HcalGeometry" << std::endl;
    cf << "geom = HcalGeometry.HcalGeometryProvider.getInstance()" << std::endl;
    cf << "gain = 0.5" << std::endl;
    cf << "hcalDigis = digi.HcalDigiProducer(gain)" << std::endl;
    cf << "hcalDigis.hgcroc.noise = False" << std::endl; //turn off noise for testing purposes
    cf << "hcalRecHits = digi.HcalRecProducer(gain)" << std::endl;
    cf << "p.sequence = [" << std::endl;
    cf << "    ldmxcfg.Producer('fakeSimHits','ldmx::test::hcal::HcalFakeSimHits','Hcal')," << std::endl;
    cf << "    hcalDigis," << std::endl;
    cf << "    hcalRecHits," << std::endl;
    cf << "    ldmxcfg.Analyzer('checkHcalHits','ldmx::test::hcal::HcalCheckEnergyReconstruction','Hcal')," << std::endl;
    cf << "    ]" << std::endl;

    /* debug printing during run
    cf << "p.termLogLevel = 1" << std::endl;
    cf << "p.logFrequency = 1" << std::endl;
     */

    /* debug pause before running
    cf << "p.pause()" << std::endl;
    */

    cf.close();

    char **args;
    ldmx::ProcessHandle p;

    ldmx::ConfigurePython cfg( config_file , args , 0 );
    REQUIRE_NOTHROW(p = cfg.makeProcess());
    p->run();
}
