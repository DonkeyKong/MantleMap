#ifndef MAPSTATE_HPP
#define MAPSTATE_HPP

#include <chrono>
#include <string>
#include <time.h>

typedef std::chrono::duration<double, std::ratio<1, 1>> fractionalSeconds;
typedef std::chrono::duration<double, std::ratio<86400, 1>> fractionalDays;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalDays > timepoint_t;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalSeconds > timepoint_seconds_t;

typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> sys_seconds;

class MapState
{
  public:
    int width;
    int height;
    int marginTop;
    int marginBottom;
    int marginLeft;
    int marginRight;
    float latitudeCenterDeg;
    float longitudeCenterDeg;
    float sunPropigationDeg;
    double homeLatitudeDeg;
    double homeLongitudeDeg;
    bool lightAdjustEnabled;
    std::string defaultScene;
    
    MapState();
    void RunTime();
    void PauseTime();
    void ResetTime();
    void GoToTime(std::tm& localTime);
    void GoToTimeRelative(fractionalDays offset);
    void GoToTimeRelative(double offsetInDays);
    void SetTimeMultiplier(double timeMultiplier);
    double GetTimeMultiplier();
    
    float GetAngleDistInDegFromHomeTangent(double latDeg, double lonDeg);
    float GetLightBoost(double sunLatDeg, double sunLonDeg);
    
    
    void SetSleep(bool value);
    bool GetSleep();
    
    timepoint_t GetMapTime();
    double GetTimeAsJulianDate();
    double GetMapTimeAsJulianDate();
    std::tm GetMapTimeAsLocaltime();
    std::tm GetLocaltime();
    double GetJulianDateFromLocaltime(std::tm&);
    std::tm GetLocaltimeFromJulianDate(double);
    
  private: 
    timepoint_t _referencePoint;
    fractionalDays _mapTimeOffset;
    bool _isSleeping;
    
    double _timeMultiplier;
    bool _timePaused;
    
};

#endif /* MAPSTATE_HPP */
