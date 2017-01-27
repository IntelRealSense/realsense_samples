# or_tutorial_2_gui

This GUI app illustrates the use of Intel® RealSense™ Cross Platform API, Intel RealSense SDK for Linux Object Library, and the Linux SDK Framework, along with RealSense camera's depth and color sensors to identify objects localization and 3d localization functionality. All items with >= 70% confidence will be identified along with the bounding box and the 3D location is being displayed on the pop up window.

# Sample output

When running the or_tutorial_2_gui sample, a pop up window will be open and the object localization result will be displayed continuously with the objects label names, recognition confidence values, and 3D coordinates.

The 3D coordinate is a 3D point representing the object location in the world.


# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_or_tutorial_2_gui
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_2_gui.

When the sample is running, click above the pop up window to run the localization algorithm on the current frame. Click again to continue the stream. 


#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
