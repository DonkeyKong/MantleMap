#pragma once

#include "Attributes.hpp"
#include <string>

bool iequals(const std::string& a, const std::string& b);

double deg2rad(double deg);

// Do an in-place normalization of an angle in degrees to
// [-180, 180]
void normalizeAngleDegrees(double& angle);

// Get a uniform random number [min,max]
int Random(int min, int max);

// Get a uniform random number [min,max]
float Random(float min, float max);

// A random position in the square specified by 
Position Random(Position min, Position max);

// A random color
Color Random(bool randomAlpha = false);

// A random HSV color, bound in HSV space
HSVColor Random(HSVColor min, HSVColor max);