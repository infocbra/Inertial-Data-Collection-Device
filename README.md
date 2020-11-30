![Inertial Data Collection Device](https://raw.githubusercontent.com/Wanghley/Inertial-Data-Collection-Device/main/images/cover.png)
![Developing](https://img.shields.io/badge/Development%20Status-in%20progress-lightgrey)
[![version](https://img.shields.io/github/v/release/Wanghley/Inertial-Data-Collection-Device?sort=semver)](https://github.com/Wanghley/Inertial-Data-Collection-Device/releases)
[![license](https://img.shields.io/github/license/Wanghley/Inertial-Data-Collection-Device)](https://github.com/RodrigoDornelles/pcb-snowman-xmas/blob/master/LICENSE.txt)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/8f7f95c491d14a2aa806aab2d97764f6)](https://www.codacy.com/gh/Wanghley/Inertial-Data-Collection-Device/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Wanghley/Inertial-Data-Collection-Device&amp;utm_campaign=Badge_Grade)

>  printed circuit board and code for collecting inertial data to research purposes of movements.

## About
Hardware developed to collect inertial data of subjects (i.e. Accelerometer, Giroscope) and temperature in order to provide an open source hardware for researchs related to the analysis of human movements for many purposes.<br>
This project was tested in the diagnosis and prognosis of neuromotor diseases - Parkinson's disease mostly as part of the project of scientific research [**"Visualization Strategies to Online Big Data applied in individualized medicine"**](https://github.com/Wanghley/PIBIC-Strategies-Data-Visualization-Medicine).

## Requirements
| Dependency | Platform | Description |
| :--------- | -------- | :---------- |
| [arduino_2012](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/arduino_2012.lbr) | eagle lib | arduino schematic and layout to **Eagle** project |
| [con-headers-jp](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/con-headers-jp.lbr) | eagle lib | schematic and layout of female pin bars to **Eagle** project |
| [DIY modules](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/diy-modules.lbr) | eagle lib | bluetooth adaptor fotoprint and symbol to **Eagle** project |
| [KRE](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/kre.lbr) | eagle lib | borne connector fotoprint and symbol to **Eagle** project |
| [pinhead](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/pinhead.lbr) | eagle lib | added in the **Eagle** project for auxiliary view of pin bars |
| [TimerOne 1.1.0](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/TimerOne-1.1.0.zip) | arduino lib | allow running a periodic interrupt function based in finer PWM control |
| [MPU6050 0.2.1](https://github.com/Wanghley/Inertial-Data-Collection-Device/blob/main/lib/MPU6050-0.2.1.zip) | arduino lib | allow MPU6050 easy control and manipulation |
| [Wire](https://www.arduino.cc/en/reference/wire) | arduino lib | allow communication over I2C protocol |
| [SoftwareSerial](https://www.arduino.cc/en/Reference/softwareSerial) | arduino lib | allow serial communication on other digital pins of the Arduino |


## Authors
*   **Wanghley Soares Martins** - *junior researcher* - [wanghley](https://github.com/wanghley)
*   **Fábio Henrique Monteiro Oliveira** - *research advisor* - [oliveirafhm](https://github.com/oliveirafhm)

See also the list of [contributors](https://github.com/Wanghley/PIBIC-Strategies-Data-Visualization-Medicine/contributors) who participated in this project.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details