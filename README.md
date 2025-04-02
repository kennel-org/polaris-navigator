# Polaris Navigator

<div style="overflow: hidden; height: auto;">
  <img src="images/logo.png" alt="Polaris Navigator Logo" style="margin: -72px 0; width: 50%;">
</div>

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
- **Compass Display**: Shows accurate heading with north indicator regardless of device orientation

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

## IMU Coordinate System and Heading Calculation

The AtomS3R IMU coordinate system is defined as follows:
- X-axis: Right direction of the device (when USB connector is facing down)
- Y-axis: Up direction of the device (when USB connector is facing down)
- Z-axis: Outward from the screen (perpendicular to the display)

The heading calculation takes into account this coordinate system and applies appropriate axis adjustments:
1. The magnetometer data is adjusted to match the device's physical orientation
2. Tilt compensation is applied using pitch and roll data to ensure accurate heading regardless of device orientation
3. The final heading is calculated using the adjusted and tilt-compensated magnetometer data

The compass display shows a red line pointing to the current heading (north) and a cyan marker indicating the position of Polaris. As you rotate the device, the red line maintains its orientation relative to north, providing an intuitive navigation reference.

### Important Notes on Compass and Heading Calculation

#### Coordinate System Considerations
- The AtomS3R's coordinate system requires careful axis mapping for correct heading calculation
- The magnetometer (BMM150) data needs to be aligned with the device's physical orientation
- Incorrect axis mapping can result in the compass needle pointing in the wrong direction

#### Heading Calculation Methods
- Simple heading calculation: `atan2(mag_y, mag_x) * 180.0 / PI`
- For proper display orientation, the angle should be negated: `-heading * PI / 180.0`
- Tilt compensation is essential for accurate heading when the device is not perfectly level

#### Polaris Display Implementation
- Polaris (North Star) should be displayed at a fixed position at the top of the compass
- The position should be independent of device rotation: `px = centerX; py = centerY - (radius * 0.7);`
- This creates a reference point that the compass needle (red line) should point to when facing north

#### Common Issues and Solutions
- If the compass needle doesn't move with device rotation, check the heading calculation in both the main sketch and IMUFusion class
- If Polaris doesn't appear, ensure it's not hidden behind conditional display logic
- For consistent behavior across different display modes, use the same angle calculation method throughout the code

## Implementation Notes

- The IMU functionality uses custom `IMUFusion`, `BMI270`, and `BMM150class` classes instead of the M5.Imu class
- The IMUFusion class provides sensor fusion algorithms for accurate orientation data
- The heading calculation uses the formula `atan2(mag_y, mag_x)` after appropriate axis adjustments and tilt compensation
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
