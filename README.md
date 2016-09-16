# Intel® RealSense™ SDK for Linux Samples

[ ![Release] [release-image] ] [releases]
[ ![License] [license-image] ] [license]

[release-image]: http://img.shields.io/badge/release-0.2.10-blue.svg?style=flat
[releases]: https://github.com/IntelRealSense/realsense_sdk

[license-image]: http://img.shields.io/badge/license-Apache--2-blue.svg?style=flat
[license]: LICENSE

## Features
These samples illustrate how to develop applications using Intel® RealSense™ cameras for Objection Recognition, Person Tracking, and SLAM.  

## Description

## What's New In This Release
Initial Release

## Compatible Devices
Intel® Linux RealSense™ 3D Camera ZR300


## Compatible Platforms
The library is written in standards-conforming C++11. It is developed and tested on the following platform:
1. Reference Linux* OS for IoT, build 181 or newer

## Supported Languages and Frameworks
C++ 

## Functionality
**API is experimental and not an official Intel product. It is subject to incompatible API changes in future updates. Breaking API changes are noted through release numbers**

The following sample functionality is provided in this release:
- **or_tutorial_1**: This console app illustrates the use of libRealSense, libOR, and the Linux SDK Framework to use the RealSense camera's depth and color sensors to identify objects in the scene. Each object identified will be printed on the command line along with the confidence value for the object, for objects which take up ~50% of the screen for recognition.
- **or_tutorial_2**: This console app builds on top of or_tutorial_1, and illustrates how to utilize the libOR object localization and 3d localization functionality. All items with >= 90% confidence will identified along with the bounding box coordinates being displayed on the console.
- **or_tutorial_3**: This console app builds on top of or_tutorial_2, and illustrates how to add the libOR object tracking functionality with localization and 3d localization. Objects for tracking can be as small as 1% of the screen.
- **pt_tutorial_1**: This console app illustrates the use of libRealsense, realsense_persontracking, and the Linux SDK Framework to use the RealSense camera's depth and color sensors to detect people in the scene. The number of people detected in the current frame as well as cumulative total number of people will be displayed as a quantity on the console.
- **pt_tutorial_2**: This sample app illustrates how to analyze someone’s posture. When a person is in the FOV, the app should display the following information: his head pose info (yaw, pitch, roll values) and his body  orientation (front/side/back values).
- **pt_tutorial_3**: This console app provides pointing gesture info for the gestures detected in the scene. This provides Pointing detected alert and also includes origin x,y coordinates as well as direction x,y coordinates.
- **slam_tutorial_1**: This console app illustrates the use of libRealsense and libSLam libraries to use the ZR300 camera's depth, fish eye, and IMU sensors to print out the current camera module position and pose

## Installation Guide
## Dependency list
The samples in this repository require librealsense, RealSense SDK framework, and RealSense Person Tracking, Object Recognition, and SLAM middleware. 

For your own reference build of Yocto, these libraries are provided in the Yocto Build layers at https://github.com/IntelRealSense/meta-intel-realsense

For a pre-built OS image for the Intel Joule module including these libraries, the instructions at https://software.intel.com/en-us/flashing-ostro-on-joule document how to ensure that your Intel Joule module is updated with the latest version of OS.  Build #181 or newer is required for these samples

## Reference Linux* OS for IoT, using Command Line
Support for building samples at the command line assume you have completed installation of the cross-compiler SDK, at the default location at /usr/local/ostroxt-x86_64.

To build all samples:
```bash
$ source /usr/local/ostroxt-x86-64/environment-setup-corei7-64-ostro-linux
$ mkdir workspace
$ cd ~/workspace
$ git clone http://github.com/IntelRealSense/realsense_samples
$ mkdir build
$ mkdir install
$ cd ~/workspace/build
$ cmake ../realsense_samples
$ make –j
$ make DESTDIR=../install install
```
This will compile all the samples using the cross-compiler, and place the sample binaries in the 'install' folder.  You can then deploy these binaries to your module using ssh.

## Reference Linux* OS for IoT, using Intel® System Studio IoT Edition
These samples are fully integrated as part of the Intel System Studio IoT Edition.  See the instructions at https://software.intel.com/en-us/node/672439 for downloading and installation steps.

## Known Issues  
n/a

## License
Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this project except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

**Other names and brands may be claimed as the property of others*
