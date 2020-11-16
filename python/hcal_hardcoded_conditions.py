""" Package to provide hard-coded conditions sources for Hcal reconstruction and simulation

Attributes
----------
HcalReconConditionsHardcode : SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal precision reconstruction
"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider

HcalReconConditionsHardcode=SimpleCSVDoubleTableProvider("HcalReconConditions",["ADC_PEDESTAL","ADC_GAIN","TOT_PEDESTAL","TOT_GAIN"])

HcalReconConditionsHardcode.validForAllRows([
    50. , #ADC_PEDESTAL - should match HgcrocEmulator
    0.5, #ADC_GAIN - 512 fC / 1024. counts - conversion to estimated charge deposited in ADC mode
    50. , #TOT_PEDESTAL - using the same pedestal as ADC right now
    2.5 #TOT_GAIN - 10240 fC / 4096 counts - conversion to estimated charge deposited in TOT mode
    ])
