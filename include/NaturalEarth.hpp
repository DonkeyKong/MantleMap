#pragma once

#include "ImageRGBA.hpp"

struct Point2D
{
    double x;
    double y;
};

class ConfigService;

class NaturalEarth
{
private:
    static double A0,A1,A2,A3,A4,B0,B1,B2,B3,B4,C0,C1,C2,C3,C4,EPS,MAX_Y;
    ConfigService& _map;
public:
    
    NaturalEarth(ConfigService& map);
    
    void map(const Point2D& in, Point2D& out);
    
    bool mapInverse(Point2D in, Point2D& out);
    
    bool mapInverse(int x, int y, Point2D& out);
    
    ImageRGBA getInvLookupTable();
    
};
