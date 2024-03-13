//
// Created by William Scheirey on 3/13/24.
//

#ifndef XBEE_BACKEND_CPP_UTILITY_H
#define XBEE_BACKEND_CPP_UTILITY_H

#pragma pack(push,1)
struct TelemPacket {
    // State Integer
    // 0 - PreLaunch
    // 1 - Launch
    // 2 - Coast
    // 3 - DrogueDescent
    // 4 - MainDescent
    // 5 - Recovery
    // 6 - Abort
    uint8_t state;
    // Raw Sensor Readings
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    float gyroX = 0.0f;
    float gyroY = 0.0f;
    float gyroZ = 0.0f;
    float magX = 0.0f;
    float magY = 0.0f;
    float magZ = 0.0f;
    float pressure = 0.0f;

    // Calculated Values
    float altitude = 0.0f;

    // EKF Results
    float w = 0.0f; // Quaternion State
    float i = 0.0f;
    float j = 0.0f;
    float k = 0.0f;
    float posX = 0.0f; // Position State ECEF
    float posY = 0.0f;
    float posZ = 0.0f;
    float velX = 0.0f; // Velocity State ECEF
    float velY = 0.0f;
    float velZ = 0.0f;

    // GPS Inputs
    float gpsLat = 0.0f;
    float gpsLong = 0.0f;
    float gpsAltMSL = 0.0f;
    float gpsAltAGL = 0.0f;
    uint32_t epochTime = 0;
    uint8_t satellites = 0;
    bool gpsLock = false;

    uint32_t timestamp = 0;
};
#pragma pack(pop)

#endif //XBEE_BACKEND_CPP_UTILITY_H


