# or_tutorial_1_gui

This GUI app illustrates the use of Intel® RealSense™ Cross Platform API, Intel RealSense SDK for Linux Object Library, and the Linux SDK Framework, along with the RealSense camera's depth and color sensors to identify objects in the scene. Each object identified will be displayed in a pop-up window with the confidence value for the object, for objects which are identified inside the painted rectangle.


# Sample output


When running the or_tutorial_1_gui sample, a pop up window will open and the object recognition result will be displayed above it.

The identified object name will be printed above the rectangle that represents the user's region of interest.
The confidence value will also appear.

If the object name is **_background**, none of the objects in the rectangle has been identified. 


# Steps to execute

Type the following command at the command prompt to execute this sample:

```
bash
$ rs_or_tutorial_1_gui
```
**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_1_gui.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
