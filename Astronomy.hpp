#ifndef ASTRONOMY_HPP
#define ASTRONOMY_HPP

extern "C"
{
  #include <novas.h>
  #include <ephutil.h>
  #include <eph_manager.h>
}

class Astronomy
{
 public:
    Astronomy();
    void Init(std::string ephemeridesPath);
    ~Astronomy();
    
    void GetSolarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg);
    void GetLunarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg);
    
  private:
    object sol, lun;
    short int accuracy;
    
    short int transit_coord(time_parameters_t* tp, object* obj,
                               double x_pole, double y_pole,
                               double* lat, double* lon);
};

#endif
