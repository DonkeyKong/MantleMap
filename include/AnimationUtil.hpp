#include <math.h>

// Animation Help Functions
template<typename T>
static void moveTowards2D(T& x, T& y, const T& targetX, const T& targetY, T velocity)
{
    // Get distance
    T dist = sqrt(pow(x-targetX,2.0) + pow(y-targetY,2.0));

    if (dist < velocity)
    { 
    x = targetX;
    y = targetY;
    }
    else
    {
    x += (targetX - x) / dist * velocity;
    y += (targetY - y) / dist * velocity;
    }
}

template<typename T>
void normAngleDeg(T& a)
{
    while (a > 180.0)
    a -= 360.0;
    while (a < -180.0)
    a += 360.0;
}

template<typename T>
T angleDiff( T angle1, T angle2 )
{
    T diff = fmod(( angle2 - angle1 + 180.0 ) , 360.0) - 180.0;
    return diff < -180.0 ? diff + 360.0 : diff;
}

template<typename T>
void moveTowardsAngleDeg2D(T& x, T& y, const T& targetX, const T& targetY, T velocity)
{
    // Get distance
    T dist = sqrt(pow(angleDiff(x, targetX),2.0) + pow(angleDiff(y, targetY),2.0));

    if (dist < velocity)
    { 
    x = targetX;
    y = targetY;
    }
    else
    {
    x += angleDiff(x, targetX) / dist * velocity;
    y += angleDiff(y, targetY) / dist * velocity;
    }
    
    normAngleDeg(x); normAngleDeg(y);
}

template<typename T>
void moveTowards(T& v, const T& targetV, T velocity)
{
    // Get distance
    T dist = abs(v-targetV);

    if (dist < velocity)
    { 
    v = targetV;
    }
    else
    {
    v += (targetV - v) / dist * velocity;
    }
}