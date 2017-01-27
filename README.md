# Intel® RealSense™ SDK for Linux Samples

[ ![Release] [release-image] ] [releases]
[ ![License] [license-image] ] [license]

[release-image]: http://img.shields.io/badge/release-0.6.5-blue.svg?style=flat
[releases]: https://github.com/IntelRealSense/realsense_sdk

[license-image]: http://img.shields.io/badge/license-Apache--2-blue.svg?style=flat
[license]: LICENSE

## Features
These samples illustrate how to develop applications using Intel® RealSense™ cameras for Object Library (OR), Person Library (PT), and Simultaneous Localization And Mapping (SLAM).

## Functionality
**API is experimental and not an official Intel product. It is subject to incompatible API changes in future updates. Breaking API changes are noted through major release numbers**

The following sample projects are provided in this release:
- **or_tutorial_1**: This console app illustrates the use of librealsense, libOR, and the Linux SDK Framework to use the ZR300 camera's depth and color sensors to identify objects in the scene. Each object identified will be printed on the command line along with the confidence value for the object, for objects which take up ~50% of the screen for recognition.
- **or_tutorial_2**: This console app builds on top of or_tutorial_1, and illustrates how to utilize the libOR object localization and 3d localization functionality. All items with >= 90% confidence will identified along with the bounding box coordinates being displayed on the console.
- **or_tutorial_3**: This console app builds on top of or_tutorial_2, and illustrates how to add the libOR object tracking functionality with localization and 3d localization. Objects for tracking can be as small as 1% of the screen.
- **or_tutorial_1_web**: This GUI app builds on top of or_tutorial_1 and displays the live color preview from the camera within a browser and shows a table with a label for the object along with its confidence value.
- **or_tutorial_2_web**: This GUI app builds on top of or_tutorial_2 and displays the live color preview from the camera within a browser and draws rectangles on the color images in the region where objects are recognized. This GUI also includes a table that shows a label for the object along with its confidence value and the 3D(x,y,z) coordinates of the object.
- **or_tutorial_3_web**: This GUI app builds on top of or_tutorial_3 and displays the live color preview from the camera within a browser and draws rectangles on the color images in the region where objects are recognized and keeps track of the objects. This GUI also includes a table that shows a label for the object along with its confidence value and the 2D(x,y) coordinates of the object.
- **or_tutorial_1_gui**: This GUI/console app builds on top of or_tutorial_1, and illustrates the use of librealsense, librealsense_object_recognition, and the Linux SDK Framework along with the RealSense camera's depth and color sensors to identify objects in the scene. Each object identified will be listed on the command line and on a pop-up window along with the confidence value for the object.
- **or_tutorial_2_gui**: This GUI/console app builds on on top of or_tutorial_2, and illustrates how to utilize object localization and 3D localization. A GUI is displayed in a pop-up window along with output on the console. Recognized objects will be highlighted with a bounding box, label, and confidence level drawn in a live video stream. Other information is displayed for each recognized object in a table in the pop-up window and on the console.
- **or_tutorial_3_gui**: This GUI/console app builds on top of or_tutorial_3, and illustrates how to add object tracking functionality with localization and 3D localization. Tracked objects can be as small as 1% of the screen. The coordinates of the tracked object(s) are displayed in a pop-up window and the console. Tracked objects are shown by a labeled bounding rectangle drawn over a live video stream. The user can trigger the localization function using the mouse.
- **or_tutorial_4_gui**: This GUI/console app builds on top of or_tutorial_3, and illustrates how to add object tracking functionality. The coordinates of the tracked object(s) are displayed in pop-up window and the console. Tracked objects are shown the pop-up window by a labeled bounding rectangle drawn over a live video stream. The user will choose the tracked object using the mouse.
- **pt_tutorial_1**: This sample app illustrates the use of libRealsense, libPT, and the Linux SDK Framework to use the ZR300 camera's depth and color sensors to detect people in the scene. The number of people detected in the scene will be displayed as a quantity on the console.
- **pt_tutorial_2**: This sample app illustrates how to analyze someone’s posture. When a person is in the FOV, the app should display the following information once every second: his head direction (yaw, pitch, roll values).
- **pt_tutorial_3**: This sample app illustrates how to get the body tracking points and detect a pointing gesture. The app will display 6 body points , a  “Pointing Detected” alert when the gesture is performed, and the pointing vector.
- **pt_tutorial_1_web**: This GUI app builds on top of pt_tutorial_1 and displays the live color preview from the camera within a browser and draw rectangles around the person(s) detected in the camera frame and draws a color dot indicating the center of mass for the detected person in the image. There is also a table as part of the GUI that shows the person id, center of mass world coordinates(x,y,z) and the cumulative count of persons detected.
- **pt_tutorial_2_web**: This GUI app builds on top of pt_tutorial_2 and displays the live color preview from the camera within a browser and draws rectangle around the head of a person detected in the camera frame. There is also a table as part of the GUI that shows the person id, and head pose information.
- **pt_tutorial_3_web**: This GUI app builds on top of pt_tutorial_3 and displays the live color preview from the camera within a browser and draws rectangle around the person detected in the camera frame. Also draws an arrow to indicate the direction of the pointing gesture. There is also a table as part of the GUI that shows the person id, gesture origin(x,y,z) and gesture direction.
- **pt_tutorial_4_web**: This GUI sample app illustrates how to register new users to the database, uploade the database to identify them when they appear in the scene.
- **pt_tutorial_5_web**:This GUI app builds on top of pt_tutorial_1, and displays live color preview of the camera, and the bounding box of all detected people in the scene. When selecting one person out of the detected, the application start tracking this person and also show his center of mass (COM) as he’s being tracked in the scene.
- **or_pt_tutorial_1**: This console app illustrates the use of librealsense, OR and PT libraries using the ZR300 camera identify objects and persons. The name of the object along with the confidence value will be printed on the console. Person information like person id and the 2D box coordinates will be printed on the console.
- **or_pt_tutorial_1_web**: This GUI app builds on top of or_pt_tutorial_1 and displays the live color preview from the camera within a browser and draw rectangles around the person(s) and object(s) detected in the camera frame. There is also a table as part of the GUI that shows the person id, center of mass world coordinates(x,y,z) along with object label, object center world coordinates and confidence score.
- **slam_tutorial_1_gui**: This app illustrates the use of the librealsense and librealsense_slam libraries. The application prints the camera pose translation to the console and draws the occupancy map in a separate window. You can pass the filename of a RealSense SDK recording as a command line argument, and it will play back the recording instead of using live camera data (see SLAM dev guide). It also illustrates how to save the occupancy map as a PPM file. This sample is the recommended starting point for learning the SLAM API.
- **slam_tutorial_1_web**: This app builds on top of slam_tutorial_1_gui and displays live fisheye preview, occupancy map, input and tracking FPS for fisheye, depth, gyro and accelerometer frames, within a browser. This application can be used for viewing the SLAM output on a remote machine, which is useful for robots and other headless systems.
- **slam_or_pt_tutorial_1**: This console app illustrates the use of librealsense, SLAM, OR and PT libraries using the ZR300 camera to print out the current camera module position and , identified objects and identified persons. The name of the object along with the confidence value will be printed on the console for identified objects. Person information like person id and the 2D box coordinates will be printed on the console.
- **slam_or_pt_tutorial_1_web**: This GUI app builds on top of slam_or_pt_tutorial_1 and displays live fisheye and color preview, occupancy map, input and tracking fps for fisheye, depth, gyro and accelerometer frames, within a browser. Draws rectangles around recognized objects and persons in the color preview. 

## Supported Languages and Frameworks
C++

## Building The Samples
Samples are provided for your convenience pre-built and validated by Intel (see: <https://software.intel.com/sites/products/realsense/intro/getting_started.html>).  In addition, samples are provided as open source, enabling you to further customize them for your specific needs. If you would like to rebuild the samples from source, follow the "Installing development Kit" section of the link above. 

## License
Copyright 2017 Intel Corporation

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
