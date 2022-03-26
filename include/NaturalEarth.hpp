#pragma once

#include <vector>

struct Point2D
{
    double x;
    double y;
};

class MapState;

class NaturalEarth
{
private:
    static double A0,A1,A2,A3,A4,B0,B1,B2,B3,B4,C0,C1,C2,C3,C4,EPS,MAX_Y;
    MapState& _map;
public:
    
    NaturalEarth(MapState& map);
    
    void map(const Point2D& in, Point2D& out);
    
    bool mapInverse(Point2D in, Point2D& out);
    
    bool mapInverse(int x, int y, Point2D& out);
    
    std::vector<unsigned char> getInvLookupTable();
    
};
