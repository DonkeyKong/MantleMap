#pragma once

#include <chrono>
#include <ctime>

typedef std::chrono::duration<double, std::ratio<1, 1>> fractionalSeconds;
typedef std::chrono::duration<double, std::ratio<86400, 1>> fractionalDays;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalDays > timepoint_t;
typedef std::chrono::time_point<std::chrono::system_clock, fractionalSeconds > timepoint_seconds_t;
typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> sys_seconds;
typedef std::chrono::high_resolution_clock::time_point FrameTimePoint;
typedef std::chrono::high_resolution_clock::duration FrameDuration;

class TimeService
{
public:
    TimeService() = delete;

    // Start scene time advancing relative to wall time
    static void RunSceneTime();

    // Stop scene time advancing relative to wall time
    static void PauseSceneTime();

    // Synchronize scene time to wall time, reset any multipliers, 
    // and set it to advance
    static void ResetSceneTime();

    // Set the scene time to a specified wall time
    static void SetSceneTime(const std::tm& localTime);

    // Get scene time as a system clock timepoint
    static timepoint_t GetSceneTime();

    // Get current scene time as a julian date
    static double GetSceneTimeAsJulianDate();

    // Get current scene time as localtime struct
    // (Useful for breaking down into months, days, etc for display)
    static std::tm GetSceneTimeAsLocaltime();

    // Increment or decrement scene time by a number of days
    static void SetSceneTimeRelative(fractionalDays offset);

    // Increment or decrement scene time by a number of days
    static void SetSceneTimeRelative(double offsetInDays);

    // Set the multiplier that describes how many scene
    // seconds advance per wall second while scene time is running
    static void SetSceneTimeMultiplier(double timeMultiplier);

    // Get the multiplier that describes how many scene
    // seconds advance per wall second while scene time is running
    static double GetSceneTimeMultiplier();
    
    // Get current wall clock time as a julian date
    static double GetWallTimeAsJulianDate();

    // Get current wall time as localtime struct
    // (Useful for breaking down into months, days, etc for display)
    static std::tm GetWallTimeAsLocaltime();

    // Convert from a localtime struct to a julian date
    static double GetJulianDateFromLocaltime(const std::tm&);

    // Convert from a julian date to a localtime struct
    static std::tm GetLocaltimeFromJulianDate(double);

    // Get the time in seconds that we would like to have between frames
    // (Generally 1/60 but can be configured)
    static double GetTargetFrameDelta();

    // Get the actual time, in seconds, that it took to render the last frame
    static double GetLastFrameDelta();

    // Record now as when the current frame finished rendering.
    // If (thisTp - lastTp) < TargetFrameDelta, wait until lastTp + TargetFrameDelta
    static void FinishAndWaitForNextFrame();
};