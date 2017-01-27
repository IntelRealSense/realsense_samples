# or_tutorial_3_gui

This GUI app illustrates the use of Intel® RealSense™ Cross Platform API, Intel RealSense SDK for Linux Object Library, and the Linux SDK Framework, along with RealSense camera's depth and color sensors to illustrates how to add the object tracking functionality with localization and 3d localization. Objects for tracking can be as small as 1% of the stream.

# Sample output

When running the or_tutorial_3_gui sample, a pop up window will be open and the object localization and tracking result will be displayed.

The localization result is displayed with the object's label name and the distance from the object (@Xm means the object is ~x meters from the camera). The sample will continue tracking these localized objects.

# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_or_tutorial_3_gui
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_3_gui.

When the sample is running, click above the pop up window to run the localization algorithm on the current frame. Click again to continue the stream. 



#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
