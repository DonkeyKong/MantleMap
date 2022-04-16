#include "AstronomyService.hpp"
#include "Utils.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <ctime>

AstronomyService::AstronomyService(ConfigService& configService) :
  config(configService)
{
  char workpath[1024];
  strncpy(workpath, configService.ephemeridesPath().c_str(), 1023);
  
  cat_entry dummy_star;
  char ttl[85];
  
  double jd_begin, jd_end;
  short int de_number;
  
  ephem_open(workpath, &jd_begin, &jd_end, &de_number);
  
  accuracy = 0;

  get_eph_title(ttl, sizeof(ttl), workpath);
  
  char entryName[] = "DUMMY";
  char entryComment[] = "xxx";
  make_cat_entry(entryName, entryComment, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, &dummy_star);
  
  make_object(0, the_planets[9].id, (char*)the_planets[9].name, &dummy_star, &sol);
  make_object(0, the_planets[10].id, (char*)the_planets[10].name, &dummy_star, &lun);
}

AstronomyService::~AstronomyService()
{
  ephem_close();
}

void AstronomyService::GetSolarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg)
{
  // Get the sun location right now
  double x_pole = 0;
  double y_pole = 0;
  time_parameters_t time;
  make_time_parameters(&time, julianDate, 0.0);
  transit_coord(&time, &sol, x_pole, y_pole, &latitudeDeg, &longitudeDeg);
  normalizeAngleDegrees(latitudeDeg);
  normalizeAngleDegrees(longitudeDeg);
}

void AstronomyService::GetLunarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg)
{
  // Get the sun location right now
  double x_pole = 0;
  double y_pole = 0;
  time_parameters_t time;
  make_time_parameters(&time, julianDate, 0.0);
  transit_coord(&time, &lun, x_pole, y_pole, &latitudeDeg, &longitudeDeg);
  normalizeAngleDegrees(latitudeDeg);
  normalizeAngleDegrees(longitudeDeg);
}

/*
 * Calculate the geodesic coordinates of the subsolar point. (Or the
 * "sub-object point", when <obj> is not Sol.)
 * This is the location on Earth at which the object is at local zenith.
 */
short int AstronomyService::transit_coord(time_parameters_t* tp, object* obj,
                               double x_pole, double y_pole,
                               double* lat, double* lon)
{
  short int error = 0;
  observer geo_ctr;
  sky_pos t_place;
  double pose[3];
  double pos[3];
  
  make_observer(0, NULL, NULL, &geo_ctr);
  if ((error = place(tp->jd_tt, obj, &geo_ctr, tp->delta_t, coord_equ, accuracy, &t_place)) != 0) 
  {
    printf("Error %d from place.", error);
  } 
  else 
  {
    radec2vector(t_place.ra, t_place.dec, t_place.dis, pose);
    if ((error = cel2ter(tp->jd_ut1, 0.0, tp->delta_t, coord_equ, accuracy, coord_equ, x_pole, y_pole, pose, pos)) != 0) 
    {
      printf("Error %d from cel2ter.", error);
    } 
    else 
    {
      vector2radec(pos, lon, lat);
      *lon *= 15.0;
      if (*lon > 180.0) 
      {
        *lon -= 360.0;
      }
    }
  }
  return error;
}


float AstronomyService::GetAngleDistInDegFromHomeTangent(double latDeg, double lonDeg)
{
  // convert to radians
  double homeLat = config.homeLatitudeDeg() * (M_PI / 180.0);
  double homeLon = config.homeLongitudeDeg() * (M_PI / 180.0);
  double lat = latDeg * (M_PI / 180.0);
  double lon = lonDeg * (M_PI / 180.0);
  
  // Generate the home and sun vectors
  double homeX = cos(homeLon)*cos(homeLat);
  double homeY = sin(homeLon)*cos(homeLat);
  double homeZ = sin(homeLat);
  double objX = cos(lon)*cos(lat);
  double objY = sin(lon)*cos(lat);
  double objZ = sin(lat);
       
  // Calculate the sun angle                
  double sunAngle = acos(homeX * objX + homeY * objY + homeZ * objZ);
  double sunAngleDeg = sunAngle * (180.0 / M_PI);
  normalizeAngleDegrees(sunAngleDeg);
  return abs(sunAngleDeg);
}

float AstronomyService::GetLightBoost(double sunLatDeg, double sunLonDeg)
{    
  // convert to radians
  double homeLat = config.homeLatitudeDeg() * (M_PI / 180.0);
  double homeLon = config.homeLongitudeDeg() * (M_PI / 180.0);
  double sunLat = sunLatDeg * (M_PI / 180.0);
  double sunLon = sunLonDeg * (M_PI / 180.0);
  double sunProp = 70 * (M_PI / 180.0);
  double sunFade = 20 * (M_PI / 180.0);
  
  // Generate the home and sun vectors
  double homeX = cos(homeLon)*cos(homeLat);
  double homeY = sin(homeLon)*cos(homeLat);
  double homeZ = sin(homeLat);
  double sunX = cos(sunLon)*cos(sunLat);
  double sunY = sin(sunLon)*cos(sunLat);
  double sunZ = sin(sunLat);

  // Calculate the sun angle                
  double sunAngle = acos(homeX * sunX + homeY * sunY + homeZ * sunZ);
  
  if (sunAngle < sunProp)
    return 1.0f;
  else if (sunAngle < (sunProp + sunFade))
    return 1.0f - ((sunAngle - sunProp) / sunFade);

  return 0.0f;
}

const double AstronomyService::EarthRadiusKm = 6371;
       
double AstronomyService::GpsDistKm(double latADeg, double lonADeg, double latBDeg, double lonBDeg)
{
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(latADeg);
  lon1r = deg2rad(lonADeg);
  lat2r = deg2rad(latBDeg);
  lon2r = deg2rad(lonBDeg);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * EarthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}