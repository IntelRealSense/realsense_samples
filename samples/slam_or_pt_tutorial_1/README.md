# slam_or_pt_tutorial_1

This console app demonstrates how to use Intel® RealSense™ Cross Platform API, Intel RealSense SDK for Linux Person, Object, and SLAM libraries, and the Linux SDK Framework with the ZR300 camera to print out the current camera module position and identified objects and identified persons. The name of the object, along with the confidence value will be printed on the console for identified objects. Person information like person ID and the 2D box coordinates will be printed on the console.

# Sample output


When running the multiple-mw-sample binary, the SLAM, object recognition and person tracking results will be printed on the console continuously.

```
tracking: accuracy=low ,	trans=(-0.11,  0.03,  0.01)
tracking: accuracy=low ,	trans=(-0.11,  0.03,  0.00)
tracking: accuracy=low ,	trans=(-0.11,  0.03, -0.01)
Current Frame Total      Cumulative
--------------------     ----------
0                        1

tracking: accuracy=low ,	trans=(-0.12,  0.03, -0.01)
tracking: accuracy=low ,	trans=(-0.12,  0.03, -0.01)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.02)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.02)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.02)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.02)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.04, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.03)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.04)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.04)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.04)
tracking: accuracy=low ,	trans=(-0.12,  0.05, -0.04)
camera pose for Object Recognition frame:(-0.11, -0.06,  0.03)
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
light             0.93              (-3.1,0.59,9.8)             (63,259) (164,295)
light             0.9               (1.2e+02,-2.1e+02,6.5e+02)  (385,11) (445,80)
pillow            0.9               (68,68,4.9e+02)             (188,173) (594,478)
light             0.89              (-1.9e+03,-7.6e+02,4.8e+03) (0,109) (126,172)
bed               0.76              (25,46,5.2e+02)             (62,114) (609,474)

```

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_slam_or_pt_tutorial_1
```
The command which will start SLAM, object recognition, and person tracking.

You can enable specific SLAM, Person Tracking, and/or Object Library processing by adding parameters [-slam, -or, -pt] singly or in combination. For example:

```
$ rs_slam_or_pt_tutorial_1 -slam -or
```

will start SLAM and object recognition.


**Note:** If you are building this sample from source, the executable name is, instead, sample_slam_or_pt_tutorial_1.


# License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
