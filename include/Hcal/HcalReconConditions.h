
#ifndef HCAL_HCALRECONCONDTIONS_H_
#define HCAL_HCALRECONCONDTIONS_H_

#include "Conditions/SimpleTableCondition.h"
#include "DetDescr/HcalID.h"

namespace ldmx {

/**
 * Class to wrap around an double table of conditions.
 *
 * This hardcodes the column numbers and checks that
 * the hardcoded numbers match the imported columns
 * during construction.
 */
class HcalReconConditions {

 public:
  /// the name of the HcalReconConditions table (must match python registration name)
  static const std::string CONDITIONS_NAME;
  /// column index for ADC pedestal
  static const unsigned int IADC_PEDESTAL    = 0;
  /// column index for ADC threshold
  static const unsigned int IADC_GAIN        = 1;
  /// column index for TOT pedestal
  static const unsigned int ITOT_PEDESTAL    = 2;
  /// column index for TOT gain
  static const unsigned int ITOT_GAIN        = 3;
  /// expected order of column names matching the above indices (must match the indices above)
  static const std::vector<std::string> EXPECTED_COLUMNS;

  /**
   * Constructor
   *
   * Assign the conditions table to use and (if validate is true),
   * check if the hard-coded column indices correctly match
   * the table that is passed.
   *
   * @raises Exception if any of the column names from the passed table
   * do not match the hardcoded indices.
   *
   * @param[in] table double table of reconstruction conditions
   * @param[in] validate true if you want to check the columns
   */
  HcalReconConditions(const DoubleTableCondition &table, bool validate = true);

  /**
   * get the ADC pedestal
   *
   * @param[in] id HCal ID for specific chip
   * @returns the ADC pedestal for that chip in counts
   */
  double adcPedestal(const HcalID& id) const {
    return the_table_.get(id.raw(), IADC_PEDESTAL);
  }

  /**
   * get the ADC gain 
   *
   * The ADC gain converts the ADC counts measuring
   * a voltage amplitude into an estimated charge
   * deposition [fC].
   *
   * @param[in] id raw ID for specific chip
   * @returns the ADC threshold for that chip in fC/counts
   */
  double adcGain(const HcalID& id) const {
    return the_table_.get(id.raw(), IADC_GAIN);
  }

  /**
   * get the TOT pedestal
   *
   * @param[in] id HCal ID for specific chip
   * @returns the TOT pedestal for that chip in counts
   */
  double totPedestal(const HcalID& id) const {
    return the_table_.get(id.raw(), ITOT_PEDESTAL);
  }

  /**
   * get the TOT gain
   *
   * The TOT gain converts counts measuring time
   * over threshold into an estimate for charge
   * deposited in that cell [fC].
   *
   * @param[in] id HCal ID for specific chip
   * @returns the TOT gain for that chip in fC/counts
   */
  double totGain(const HcalID& id) const { 
    return the_table_.get(id.raw(), ITOT_GAIN); 
  }

 private:
  /// reference to the table of conditions storing the chip conditions
  const DoubleTableCondition &the_table_;
}; // HcalReconConditions

} // namespace ldmx

#endif // HCAL_HCALRECONCONDITIONS_H_
