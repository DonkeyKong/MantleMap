#pragma once

#include "ConfigService.hpp"

extern "C"
{
  #include <novas.h>
  #include <ephutil.h>
  #include <eph_manager.h>
}

#include <string>

class AstronomyService
{
 public:
    AstronomyService(ConfigService&);
    ~AstronomyService();
    
    void GetSolarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg);
    void GetLunarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg);

    // Light angle utilities
    float GetAngleDistInDegFromHomeTangent(double latDeg, double lonDeg);
    float GetLightBoost(double sunLatDeg, double sunLonDeg);

    static const double EarthRadiusKm;
    static double GpsDistKm(double latADeg, double lonADeg, double latBDeg, double lonBDeg);
    
  private:
    object sol, lun;
    short int accuracy;
    ConfigService& config;
    
    short int transit_coord(time_parameters_t* tp, object* obj,
                               double x_pole, double y_pole,
                               double* lat, double* lon);
};
