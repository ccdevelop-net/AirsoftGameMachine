#ifndef NEOGPS_LOCATION_H
#define NEOGPS_LOCATION_H

#include <cstdint>
#include <cstdbool>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>

#include <gps-config.hpp>



class NMEAGPS;

namespace Airsoft::Neo {

constexpr float LOC_SCALE { 1.0e-7 };

constexpr float EARTH_RADIUS_KM { 6371.0088 };
constexpr float RAD_PER_DEG     { M_PI / 180.0 };
constexpr float DEG_PER_RAD     { 180.0 / M_PI };
constexpr float MI_PER_KM       { 0.621371 };


class Location {
public:

    Location() = default;
    Location(int32_t lat, int32_t lon) : _lat(lat), _lon(lon) {}
    Location(float lat, float lon) : _lat(lat / LOC_SCALE), _lon(lon / LOC_SCALE) {}
    Location(double lat, double lon) : _lat(lat / LOC_SCALE), _lon(lon / LOC_SCALE) {}

    int32_t Lat(void) const {
      return _lat;
    }
    void Lat(int32_t l) {
      _lat = l;
    }
    float LatF(void) const {
      return static_cast<float>(Lat()) * LOC_SCALE;
    }
    void LatF(float v) {
      _lat = v / LOC_SCALE;
    }

    int32_t Lon(void) const {
      return _lon;
    }
    void Lon(int32_t l) {
      _lon = l;
    }
    float LonF(void) const {
      return static_cast<float>(Lon()) * LOC_SCALE;
    }
    void LonF(float v) {
      _lon = v / LOC_SCALE;
    }

    //-----------------------------------
    // Distance calculations

    static float DistanceKm(const Location & p1, const Location & p2 ) {
      return DistanceRadians(p1, p2) * EARTH_RADIUS_KM;
    }
    float DistanceKm(const Location & p2) const {
      return DistanceKm(*this, p2);
    }

    static float DistanceMiles(const Location & p1, const Location & p2) {
      return DistanceRadians( p1, p2 ) * EARTH_RADIUS_KM * MI_PER_KM;
    }
    float DistanceMiles(const Location & p2) const {
      return DistanceMiles(*this, p2);
    }

    static float DistanceRadians(const Location & p1, const Location & p2);
    float DistanceRadians(const Location & p2) const {
      return DistanceRadians(*this, p2);
    }

    static float EquirectDistanceRadians(const Location & p1, const Location & p2);
    float EquirectDistanceRadians(const Location & p2) const {
      return EquirectDistanceRadians(*this, p2);
    }

    static float EquirectDistanceKm(const Location & p1, const Location & p2) {
      return EquirectDistanceRadians( p1, p2 ) * EARTH_RADIUS_KM;
    }
    float EquirectDistanceKm(const Location & p2) const {
      return EquirectDistanceKm(*this, p2);
    }

    static float EquirectDistanceMiles(const Location & p1, const Location & p2) {
      return EquirectDistanceRadians(p1, p2) * EARTH_RADIUS_KM * MI_PER_KM;
    }
    float EquirectDistanceMiles(const Location & p2) const {
      return EquirectDistanceMiles(*this, p2);
    }

    // Bearing calculations
    static float BearingTo(const Location & p1, const Location & p2); // radians
    float BearingTo(const Location & p2) const {
      return BearingTo(*this, p2);
    }

    static float BearingToDegrees(const Location & p1, const Location & p2) {
      return BearingTo(p1, p2) * DEG_PER_RAD;
    }
    float BearingToDegrees(const Location & p2) const {
      return BearingToDegrees(*this, p2);
    }

    // Offset a location (note distance is in radians, not degrees)
    void OffsetBy(float distR, float bearingR);

//private: //---------------------------------------
    friend class NMEAGPS; // This does not work?!?

    int32_t       _lat {};  // degrees * 1e7, negative is South
    int32_t       _lon {};  // degrees * 1e7, negative is West

};

} // namespace Airsoft::Neo

#endif
