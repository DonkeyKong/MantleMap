//
//  NaturalEarthII.cpp
//  MantlemapProjectionTest
//
//  Created by Zack Schilling on 1/12/19.
//  Copyright © 2019 Bubbulon. All rights reserved.
//

#include "NaturalEarth.hpp"
#include "MapState.hpp"

#include <math.h>

double NaturalEarth::A0 = 0.8707;
double NaturalEarth::A1 = -0.131979;
double NaturalEarth::A2 = -0.013791;
double NaturalEarth::A3 = 0.003971;
double NaturalEarth::A4 = -0.001529;
double NaturalEarth::B0 = 1.007226;
double NaturalEarth::B1 = 0.015085;
double NaturalEarth::B2 = -0.044475;
double NaturalEarth::B3 = 0.028874;
double NaturalEarth::B4 = -0.005916;
double NaturalEarth::C0 = B0;
double NaturalEarth::C1 = 3 * B1;
double NaturalEarth::C2 = 7 * B2;
double NaturalEarth::C3 = 9 * B3;
double NaturalEarth::C4 = 11 * B4;
double NaturalEarth::EPS = 1e-11;
double NaturalEarth::MAX_Y = 0.8707 * 0.52 * M_PI;

NaturalEarth::NaturalEarth(MapState& map) : _map(map)
{
  
}

void NaturalEarth::map(const Point2D& in, Point2D& out)
{
    double phi2 = in.y * in.y;
    double phi4 = phi2 * phi2;
    
    // Apply offset and normalize angles
    double x = in.x - (_map.longitudeCenterDeg * (M_PI / 180.0));
    while (x < -M_PI)
      x += M_PI * 2.0;
    while (out.x > M_PI)
      x -= M_PI * 2.0;
    
    out.x =    x * (A0 + phi2 * (A1 + phi2 * (A2 + phi4 * phi2 * (A3 + phi2 * A4))));
    out.y = in.y * (B0 + phi2 * (B1 + phi4 * (B2 + B3 * phi2 + B4 * phi4)));
}

bool NaturalEarth::mapInverse(Point2D in, Point2D& out)
{
    bool yoob = false;
    // make sure y is inside valid range
    if (in.y > MAX_Y)
    {
      yoob = true;
      in.y = MAX_Y;
    }
    else if (in.y < -MAX_Y)
    {
      yoob = true;
      in.y = -MAX_Y;
    }
        
    // latitude
    double yc = in.y;
    double tol;
    for (;;) { // Newton-Raphson
        double y2 = yc * yc;
        double y4 = y2 * y2;
        double f = (yc * (B0 + y2 * (B1 + y4 * (B2 + B3 * y2 + B4 * y4)))) - in.y;
        double fder = C0 + y2 * (C1 + y4 * (C2 + C3 * y2 + C4 * y4));
        yc -= tol = f / fder;
        if (abs(tol) < EPS) {
            break;
        }
    }
    out.y = yc;
    
    // longitude
    double y2 = yc * yc;
    double phi = A0 + y2 * (A1 + y2 * (A2 + y2 * y2 * y2 * (A3 + y2 * A4)));
    out.x = in.x / phi;
    
    // Trim values outside sphere
    bool wrapping = out.x > M_PI || out.x < -M_PI || out.y > M_PI || out.y < -M_PI;
    
    // Add offset
    out.x += (_map.longitudeCenterDeg * (M_PI / 180.0));
    
    // Normalize angles
    while (out.x < -M_PI)
      out.x += M_PI * 2.0;
    while (out.x > M_PI)
      out.x -= M_PI * 2.0;
      
    while (out.y < -M_PI)
      out.y += M_PI * 2.0;
    while (out.y > M_PI)
      out.y -= M_PI * 2.0;
      
    return !wrapping && !yoob;
}

bool NaturalEarth::mapInverse(int x, int y, Point2D& out)
{
  double scaledFromPixel = (MAX_Y*2.0) / (double)(_map.height - _map.marginTop - _map.marginBottom + 1);
  double max_x = ((double)(_map.width-_map.marginLeft-_map.marginRight) * scaledFromPixel) / 2.0;
   
  Point2D scaled;
  scaled.x = (scaledFromPixel * (x-_map.marginLeft)) - max_x;
  scaled.y = MAX_Y - (scaledFromPixel * (y-_map.marginTop));
  return mapInverse(scaled, out);
}

ImageRGBA NaturalEarth::getInvLookupTable()
{
  ImageRGBA lut(_map.width , _map.height);

  Point2D lonLat;
  for (int y=0; y < _map.height; y++)
  {
    for (int x=0; x < _map.width; x++)
    {
      int i = (y * _map.width + x)*4;
      bool valid = mapInverse(x, y, lonLat);

      float v1 = cos(lonLat.x)*cos(lonLat.y);
      float v2 = sin(lonLat.x)*cos(lonLat.y);
      float v3 = sin(lonLat.y);
      float scale = sqrt(pow(v1,2.0) + pow(v2,2.0) + pow(v3,2.0)) * 127;
      
      lut[i+0] = v1 * scale + 128;
      lut[i+1] = v2 * scale + 128;
      lut[i+2] = v3 * scale + 128;
      lut[i+3] = valid ? 255 : 0;
    }
  }
  return lut;
}