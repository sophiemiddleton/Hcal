#include "DetDescr/HcalReadout.h"
#include "Framework/ConditionsObjectProvider.h"
#include "Event/RunHeader.h"
#include "Event/EventHeader.h"
#include <sys/types.h>
#include <regex.h>

/**
 * @file HcalGeometryProvider.cxx
 * @brief Class that creates HcalReadout object based on python specification
 * @author Jeremiah Mans, UMN
 */

namespace ldmx {

class HcalGeometryProvider : public ConditionsObjectProvider {
 public:
  /**
   * Class constructor
   * @param parameters -- uses the "HcalReadout" section to configure the HcalReadout
   */	
  HcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process);


  /** Destructor */
  virtual ~HcalGeometryProvider();
	
  /**
   * Provides access to the HcalReadout 
   * @note Currently, these are assumed to be valid for all time, but this behavior could be changed.  Users should not cache the pointer
   * between events
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context);

  /**
   * Take no action on release, as the object is permanently owned by the Provider
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) {
  }

  virtual void onNewRun(RunHeader& rh) {
    if (detectorGeometry_.empty()) detectorGeometry_=rh.getDetectorName();
    else if (hcalGeometry_!=nullptr && detectorGeometry_!=rh.getDetectorName()) {
      EXCEPTION_RAISE("GeometryException","Attempting to run a single job with multiple geometries "+detectorGeometry_+" and '"+rh.getDetectorName()+"'");
    }
    //make sure detector name has been set
    if (detectorGeometry_.empty())
        EXCEPTION_RAISE("GeometryException","HcalGeometryProvider unable to get the name of the detector from the RunHeader.");
  }

    
 private:
  /** Handle to the parameters, needed for future use during get condition */
  Parameters params_;
  /** Geometry as last used */
  std::string detectorGeometry_;
  HcalReadout* hcalGeometry_;
};
    

HcalGeometryProvider::HcalGeometryProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) : ConditionsObjectProvider{HcalReadout::CONDITIONS_OBJECT_NAME,tagname,parameters,process}, params_{parameters} {
      
  hcalGeometry_=0;
}

HcalGeometryProvider::~HcalGeometryProvider() {
  if (hcalGeometry_) delete hcalGeometry_;
  hcalGeometry_=0;
}

std::pair<const ConditionsObject*,ConditionsIOV> HcalGeometryProvider::getCondition(const EventHeader& context) {
  static const std::string KEYNAME("detectors_valid");
	
  if (!hcalGeometry_) {

    Parameters phex=(params_.exists("HcalReadout"))?(params_.getParameter<Parameters>("HcalReadout")):(params_);
    
    // search through the subtrees
    for (auto key: phex.keys()) {
      Parameters pver=phex.getParameter<Parameters>(key);
      
      if (!pver.exists(KEYNAME)) {
        ldmx_log(warn) << "No parameter " << KEYNAME << " found in " << key;
        // log strange situation and continue
        continue;
      }
      
     std::vector<std::string> dets_valid=pver.getParameter<std::vector<std::string> >(KEYNAME);
      for (auto detregex : dets_valid) {
        std::string regex(detregex);
        if (regex.empty()) continue; // no empty regex allowed
        if (regex[0]!='^') regex.insert(0,1,'^');
        if (regex.back()!='$') regex+='$';
        regex_t reg;
        
	
        int rv=regcomp(&reg,regex.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB);
        if (rv) {
          char err[1024];
          regerror(rv,&reg,err,1024);
          EXCEPTION_RAISE("GeometryException","Invalid detector regular expression : '"+regex+"' "
                          +err);
        }
        int nmatch=regexec(&reg,detectorGeometry_.c_str(),0,0,0);
        regfree(&reg);
        if (!nmatch) {
          hcalGeometry_=new HcalReadout(pver);
          break;
        }
      }
      if (hcalGeometry_) break;
      
    }
    if (!hcalGeometry_) {
      EXCEPTION_RAISE("GeometryException","Unable to create HcalReadout");
    }
    
  }
    
  return std::make_pair(hcalGeometry_,ConditionsIOV(context.getRun(),context.getRun(),true,true));
}

} // namespace ldmx

DECLARE_CONDITIONS_PROVIDER_NS(ldmx, HcalGeometryProvider);
