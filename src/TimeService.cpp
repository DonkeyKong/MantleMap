#include "TimeService.hpp"

struct Impl
{
    // Internal time state vars
    timepoint_t referencePoint;
    fractionalDays mapTimeOffset;
    double timeMultiplier;
    bool timePaused;

    Impl()
    {
        // Internal vars
        timeMultiplier = 1.0;
        timePaused = false;
        TimeService::ResetSceneTime();
    }
};

static Impl impl;

static double getJulianFromUnix( double unixSecs )
{
  return (unixSecs / 86400.0 ) + 2440587.5;
}

static double getUnixFromJulian( double julian )
{
  return (julian - 2440587.5) * 86400.0;
}

// Start scene time advancing relative to wall time
void TimeService::RunSceneTime()
{
    // Set _referencePoint to now
    // Adjust _mapTimeOffset so maptime doesn't change
    // mark the stopped boolean to false

    timepoint_t currMapTime = GetSceneTime();
    impl.referencePoint = std::chrono::system_clock::now();
    impl.mapTimeOffset =  currMapTime - impl.referencePoint;
    impl.timePaused = false;
}

// Stop scene time advancing relative to wall time
void TimeService::PauseSceneTime()
{
    // Set _referencePoint to now
    // Adjust _mapTimeOffset so maptime doesn't change
    // mark the stopped boolean true

    timepoint_t currMapTime = GetSceneTime();
    impl.referencePoint = std::chrono::system_clock::now();
    impl.mapTimeOffset =  currMapTime - impl.referencePoint;
    impl.timePaused = true;
}

// Synchronize scene time to wall time, reset any multipliers, 
// and set it to advance
void TimeService::ResetSceneTime()
{
    // Set _mapTimeOffset = 0 and _referencePoint = now
    impl.timeMultiplier = 1.0;
    impl.mapTimeOffset = fractionalDays();
    impl.referencePoint = std::chrono::system_clock::now();
}

// Set the scene time to a specified wall time
void TimeService::SetSceneTime(const std::tm& localTime)
{
    std::tm lt = localTime;
    // Convert localTime structure to time_t
    time_t localTime_t = mktime(&lt);

    // Convert the time_t to a time point
    timepoint_t localTime_tp = std::chrono::system_clock::from_time_t(localTime_t);

    // Set referencePoint to now and _mapTimeOffset to the difference between the points
    impl.referencePoint = std::chrono::system_clock::now();
    impl.mapTimeOffset = localTime_tp - impl.referencePoint;
}

// Get scene time as a system clock timepoint
timepoint_t TimeService::GetSceneTime()
{
    // Map time is the difference between reference time and now, 
    // multiplied by the time multiplier,
    // and added to the mapReferenceTime

    if (impl.timePaused)
    {
        return impl.referencePoint + impl.mapTimeOffset;
    }
    else
    {
        timepoint_t now = std::chrono::system_clock::now();
        return impl.referencePoint + impl.mapTimeOffset + ((now - impl.referencePoint) * impl.timeMultiplier);
    }
}

// Get current scene time as a julian date
double TimeService::GetSceneTimeAsJulianDate()
{
    return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(GetSceneTime().time_since_epoch()).count());
}

// Get current scene time as localtime struct
// (Useful for breaking down into months, days, etc for display)
std::tm TimeService::GetSceneTimeAsLocaltime()
{
    timepoint_t mapTime = GetSceneTime(); //<class ToDuration, class Clock, class Duration>
    sys_seconds tp = std::chrono::time_point_cast<std::chrono::seconds>(mapTime);
    std::time_t currTime_t = std::chrono::system_clock::to_time_t( tp );
    return *localtime(&currTime_t);
}

// Increment or decrement scene time by a number of days
void TimeService::SetSceneTimeRelative(fractionalDays offset)
{
    impl.mapTimeOffset += offset;
}

// Increment or decrement scene time by a number of days
void TimeService::SetSceneTimeRelative(double offsetInDays)
{
    SetSceneTimeRelative(fractionalDays(offsetInDays));
}

// Set the multiplier that describes how many scene
// seconds advance per wall second while scene time is running
void TimeService::SetSceneTimeMultiplier(double timeMultiplier)
{
    timepoint_t currMapTime = GetSceneTime();
    impl.referencePoint = std::chrono::system_clock::now();
    impl.mapTimeOffset =  currMapTime - impl.referencePoint;
    impl.timeMultiplier = timeMultiplier;
}

// Get the multiplier that describes how many scene
// seconds advance per wall second while scene time is running
double TimeService::GetSceneTimeMultiplier()
{
    return impl.timeMultiplier;
}

// Get current wall clock time as a julian date
double TimeService::GetWallTimeAsJulianDate()
{
    timepoint_t now = std::chrono::system_clock::now();
    return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(now.time_since_epoch()).count());
}

// Get current wall time as localtime struct
// (Useful for breaking down into months, days, etc for display)
std::tm TimeService::GetWallTimeAsLocaltime()
{
    const auto currTime = std::chrono::system_clock::now();
    std::time_t currTime_t = std::chrono::system_clock::to_time_t( currTime );
    return *localtime(&currTime_t);
}

// Convert from a localtime struct to a julian date
double TimeService::GetJulianDateFromLocaltime(const std::tm& localTime)
{
    std::tm lt = localTime;
    // Convert localTime structure to time_t
    time_t localTime_t = mktime(&lt);

    // Convert the time_t to a time point
    timepoint_t localTime_tp = std::chrono::system_clock::from_time_t(localTime_t);

    return getJulianFromUnix(std::chrono::duration_cast<fractionalSeconds>(localTime_tp.time_since_epoch()).count());
}

// Convert from a julian date to a localtime struct
std::tm TimeService::GetLocaltimeFromJulianDate(double julian)
{
    double unixDouble = getUnixFromJulian(julian);
    time_t unixTimeT = unixDouble;
    return *localtime(&unixTimeT);
}

// Get the time in seconds that we would like to have between frames
// (Generally 1/60 but can be configured)
double TimeService::GetTargetFrameDelta()
{
    return 0;
}

// Get the actual time, in seconds, that it took to render the last frame
double TimeService::GetLastFrameDelta()
{
    return 0;
}

// Record now as when the current frame finished rendering.
// If (thisTp - lastTp) < TargetFrameDelta, wait until lastTp + TargetFrameDelta
void TimeService::FinishAndWaitForNextFrame()
{
    
}


//   // Use this regex for delta time queries
//   {
//     std::smatch match; 
//     std::regex deltaTimeRegex(R"((?:move|go|travel|show me)\s+(forwards?|backwards?|back)?(\s+(?:and\s+)?(-?[0-9]+(?:\.)?[0-9]*)\s+(minutes?|hours?|days?|weeks?|months?))+)", std::regex_constants::icase);
//     if (std::regex_match(query, match, deltaTimeRegex))
//     {
//       double dir = 1;
//       double delta = 0;
//       // Captured groups are:
//       // 1 - backward
//       // 2 - and 38 minutes
//       // 3 - 38
//       // 4 - minutes
//       // ...(2,3,4 repeating)
      
//      if (match[1].str()[0] == 'b')
//         dir = -1;
      
//       for (int i=3; i < (int)match.size(); i+=3)
//       {
//         if (match[i+1].str().rfind("mi", 0) == 0)
//         {
//           delta += std::stod(match[i]) / 24.0 / 60.0;
//         }
//         else if (match[i+1].str().rfind("h", 0) == 0)
//         {
//           delta += std::stod(match[i]) / 24.0;
//         }
//         else if (match[i+1].str().rfind("d", 0) == 0)
//         {
//           delta += std::stod(match[i]);
//         }
//         else if (match[i+1].str().rfind("w", 0) == 0)
//         {
//           delta += std::stod(match[i]) * 7.0;
//         }
//         else if (match[i+1].str().rfind("mo", 0) == 0)
//         {
//           delta += round(std::stod(match[i]) * (365.0 / 12.0));
//         }
//       }
      
//       TimeService::SetSceneTimeRelative(delta * dir);

//       std::stringstream output;
//       output << "Time adjustment adjusted by " << delta * dir << " days" << std::endl;
//       response = output.str();
    
//       return true;
//     }
//   }
  
//   // Use this regex for time multiplier queries
//   {
//     std::smatch match; 
//     std::regex deltaTimeRegex(R"(set\s+(?:time speed|time|speed)\s+multiplier\s+(?:to\s+)?(-?[0-9]+(?:\.)?[0-9]*))", std::regex_constants::icase);
//     if (std::regex_match(query, match, deltaTimeRegex))
//     {
//       double timeMultiplier = 1.0;
     
//       timeMultiplier = std::stod(match[1]);
      
//       TimeService::SetSceneTimeMultiplier(timeMultiplier);
      
//       std::stringstream output;
//       output << "Speed multiplier set to " << timeMultiplier << " days" << std::endl;
//       response = output.str();
    
//       return true;
//     }
//   }