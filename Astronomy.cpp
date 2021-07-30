#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <ctime>

#include "Astronomy.hpp"

Astronomy::Astronomy()
{
  char workpath[] = "/home/pi/mantlemap/de430/linux_p1550p2650.430";
  
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

Astronomy::~Astronomy()
{
  ephem_close();
}

static void normalizeAngleDegrees(double& angle)
{
  while (angle < -180.0)
    angle += 360.0;
  while (angle > 180.0)
    angle -= 360.0;
}

void Astronomy::GetSolarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg)
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

void Astronomy::GetLunarPoint(double julianDate, double& latitudeDeg, double& longitudeDeg)
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
short int Astronomy::transit_coord(time_parameters_t* tp, object* obj,
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