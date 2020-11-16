"""Package to configure the HCal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
nPEPerMIP: float
    Number of PE created for each MIP for a 20 mm scintillator tile
mipSiEnergy: float
    Energy [MeV] of a single MIP on average in 20 mm scintillator
"""

import numpy as np
from LDMX.Framework.ldmxcfg import Producer

nPEPerMIP = 68. #PEs created per MIP 
mipSiEnergy = 4.66 #MeV - measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6      

def HcalHgcrocEmulator(gain) :
    """Get an HGCROC emulator and configure for the HCal specifically

    This sets the pulse shape parameters to the ones from a fit 
    to a test readout of an HCal module and then thresholds to 
    1 PE using 68 as the number of PEs per MIP.
    """

    from LDMX.Tools import HgcrocEmulator
    hgcroc = HgcrocEmulator.HgcrocEmulator()

    # readout capacitance of chip is ~20pF
    hgcroc.readoutPadCapacitance = 20. #pF  
    
    # set gain and tot/toa thresholds
    hgcroc.setThresholdDefaultsHcal( gain )

    # set pulse shape parameters                                                                                                                                              
    hgcroc.rateUpSlope = -0.1141
    hgcroc.timeUpSlope = -9.897
    hgcroc.rateDnSlope = 0.0279
    hgcroc.timeDnSlope = 45.037
    hgcroc.timePeak    = 9.747
    
    # try ecal params 
#     hgcroc.rateUpSlope =  -0.345
#     hgcroc.timeUpSlope = 70.6547
#     hgcroc.rateDnSlope = 0.140068
#     hgcroc.timeDnSlope = 87.7649
#     hgcroc.timePeak    = 77.732

    return hgcroc

class HcalDigiProducer(Producer) :
    """Configuration for HcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    MeV : float
        Conversion between energy [MeV] and voltage [mV]
    nHcalLayers : int
        Number of scintillator layer in HCal, needed to generate noise ID
    nModulesPerLayer : int
        Number of modules in each layer, needed to generate noise ID
    nCellsPerModule : int
        Number of cells in each module, needed to generate noise ID
    """

    def __init__(self, gain, instance_name = 'hcalDigis') :
        super().__init__(instance_name , 'ldmx::HcalDigiProducer','Hcal')

        self.hgcroc = HcalHgcrocEmulator(gain)

        #Energy -> PE converstion
        # energy [MeV] ( 1 MIP / energy per MIP [MeV] ) ( voltage per MIP [mV] / 1 MIP ) = voltage [mV]                                                                   
        # assuming 7 PEs ~ 2.5V ~ 2500 mV -> self.MeV = 3.64 V/MeV  ~ 3640 mV/MeV
        # assuming 1 PEs ~ 5mV ->  self.MeV = 72.961 mV/MeV (current)
        self.MeV = (1./mipSiEnergy)*self.hgcroc.calculateVoltageHcal( nPEPerMIP )

class HcalRecProducer(Producer) :
    """Configuration for the HcalRecProducer

    Attributes
    ----------
    voltage_per_mip: float
        Conversion from voltage [mV] to number of MIPs
    mip_si_energy : float
        Copied from module-wide mipSiEnergy [MeV]
    clock_cycle : float
        Time for one DAQ clock cycle to pass [ns]
    digiCollName : str
        Name of digi collection
    digiPassName : str
        Name of digi pass
    secondOrderEnergyCorrection : float
        Correction to weighted energy
    layerWeights : list of floats
        Weighting factors depending on layer index
    """

    def __init__(self, gain, instance_name = 'hcalRecon') :
        super().__init__(instance_name , 'ldmx::HcalRecProducer','Hcal')

        self.voltage_per_mip = (5/1)*(nPEPerMIP) # 5*68 mV/ MIP
        self.mip_si_energy = mipSiEnergy #MeV / MIP
        self.clock_cycle = 25. #ns - needs to match the setting on the chip

        self.digiCollName = 'HcalDigis'
        self.digiPassName = ''
        self.simHitCollName = 'HcalSimHits'
        self.simHitPassName = ''
        self.recHitCollName = 'HcalRecHits'

        # geometry dependent settings
        self.secondOrderEnergyCorrection = 1.
        self.layerWeights = [ ]

        #from LDMX.DetDescr import HcalHexReadout
        #self.hexReadout = HcalHexReadout.HcalHexReadout()

        self.v12() #use v12 geometry by default

    def v12(self) :
        """These layerWeights and energy correction were calculated for the v12 geometry.
        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        muon events.
        """
        #self.hexReadout.v12()
        self.secondOrderEnergyCorrection = 4000./4010.;
        self.layerWeights = list(np.ones(101, dtype=np.double));
