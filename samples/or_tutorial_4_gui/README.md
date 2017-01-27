# or_tutorial_4_gui

This GUI app demonstrates the use of Intel® RealSense™ Cross Platform API, Intel RealSense SDK for Linux Object Library, and the Linux SDK Framework, along with RealSense camera's depth and color sensors to show how to add object tracking functionality. Objects for tracking can be as small as 1% of the stream.

# Sample output

When running the or_tutorial_4_gui sample, a pop up window will be open. 

The user can select the object to track by using the mouse (by clicking and dragging the left button), then the object tracking result will be displayed.
The sample will continue tracking the object until the user selects another object to track.

# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_or_tutorial_4_gui
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_4_gui.

When the sample is running, click above the pop up window to run the localization algorithm on the current frame. Click again to continue the stream. 



#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
