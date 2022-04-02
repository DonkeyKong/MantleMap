#include "Utils.hpp"

#include <string>
#include <algorithm>
#include <ctype.h>
#include <stdlib.h>

bool iequals(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}

// Get a uniform random number [min,max]
int Random(int min, int max)
{
    return min + rand() % (max-min+1);
}

// Get a uniform random number [min,max]
float Random(float min, float max)
{
    return min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
}

// A random position in the square specified by 
Position Random(Position min, Position max)
{
    return 
    {
        Random(min.x, max.x),
        Random(min.y, max.y),
        Random(min.z, max.z)
    };
}

// A random color
Color Random(bool randomAlpha)
{
    return 
    {
        Random(0.0f, 1.0f),
        Random(0.0f, 1.0f),
        Random(0.0f, 1.0f),
        randomAlpha ? Random(0.0f, 1.0f) : 1.0f
    };
}

// A random HSV color, bound in HSV space
HSVColor Random(HSVColor min, HSVColor max)
{
    return 
    {
        Random(min.h, max.h),
        Random(min.s, max.s),
        Random(min.v, max.v),
        Random(min.a, max.a)
    };
}
