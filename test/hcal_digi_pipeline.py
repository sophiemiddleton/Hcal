#!/bin/python3

from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("v12")
p.run = 1

import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions

from LDMX.SimCore import simulator
sim = simulator.simulator("mySim")
sim.setDetector( 'ldmx-det-v12', True  )
sim.description = "HCal Digi Pipeline Tested on Basic 4GeV Gun"
from LDMX.SimCore import generators
particle_gun = generators.gun( "single_4gev_n_upstream_target")
particle_gun.particle = 'neutron'
particle_gun.position = [ 0., 0., -1.2 ]  # mm
particle_gun.direction = [ 0., 0., 1]
energy = 4.
particle_gun.energy = energy #GeV 
sim.generators = [ particle_gun ]
sim.beamSpotSmear = [80., 80., 0]
from LDMX.Hcal import digi
gain = 0.5
hcaldigi = digi.HcalDigiProducer(gain,'myHcalDigis')
hcaldigi.hgcroc.gain = gain
hcalrec = digi.HcalRecProducer(gain,'myHcalRecHits')
p.sequence=[ sim, 
             hcaldigi, 
             hcalrec,
             ]
p.outputFiles=["tmp/hcal_digi_pipeline.root"]
p.maxEvents = 1
