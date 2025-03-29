# Polaris Navigator

A polar alignment assistant device for astrophotography using AtomS3R (with IMU) and AtomicBase GPS.

## Overview

Polaris Navigator is a compact, portable device designed to assist astrophotographers with polar alignment. It uses IMU (Inertial Measurement Unit) and GPS data to visualize the alignment error between your telescope/camera and the celestial pole (North/South).

## Features

- **Polar Alignment Assistance**: Calculates and displays azimuth and altitude errors relative to the celestial pole
- **Altitude Indicator**: Visual representation of target altitude (Polaris) and current device pitch
- **Celestial Object Tracking**: Shows the direction of the sun and moon
- **Moon Phase Display**: Calculates and displays the current moon phase
- **GPS Data Display**: Shows raw GPS data including latitude, longitude, and satellite status
- **IMU Data Display**: Displays raw accelerometer, gyroscope, and magnetometer data for calibration
- **IMU Calibration**: Provides a guided calibration process for the magnetometer and accelerometer/gyroscope sensors

## Hardware

- AtomS3R (with built-in IMU)
  - BMI270 (6-axis accelerometer/gyroscope)
  - BMM150 (3-axis magnetometer)
- AtomicBase GPS
- Display device (built-in LCD or OLED)

## IMU Calibration

The AtomS3R features a 9-axis sensor system consisting of the BMI270 (6-axis accelerometer/gyroscope) and BMM150 (3-axis magnetometer). The calibration screen provides color-coded status indicators for sensor calibration and guides users through the calibration process:

1. For magnetometer calibration: Rotate the device in a figure-8 pattern
2. For accelerometer/gyroscope calibration: Flip the device in all directions

Calibration status is displayed in three levels (GOOD, OK, POOR) and shows "NOT CALIBRATED" when sensors are uncalibrated.

## Implementation Notes

- The IMU functionality uses custom `IMUFusion`, `BMI270`, and `BMM150class` classes instead of the M5.Imu class
- The IMUFusion class provides sensor fusion algorithms for accurate orientation data
- GPS communication uses pins: TX = 5, RX = -1 (when using AtomicBase GPS)
- GPS data is saved to flash memory and reused when GPS signal is unavailable
- The UI is optimized for the small AtomS3R display with clear indicators for alignment

## Usage

### Polar Alignment Mode
1. The main screen shows a compass with the current heading and Polaris position
2. Align the red line (current heading) with the cyan star marker (Polaris)
3. Use the altitude indicator to match the current pitch (yellow triangle) with the target altitude (cyan marker)
4. When both azimuth and altitude are aligned, your mount is properly polar aligned

### Raw Data Modes
Press the button to cycle through different data display modes:
- IMU data
- GPS data
- Celestial data
- System information
- Debug information

## Development Status

This project is currently under development.

## License

[TBD]
