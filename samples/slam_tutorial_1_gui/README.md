# slam_tutorial_1_gui

This app illustrates the use of the Intel® RealSense™ Cross Platform API and Intel RealSense SDK for Linux SLAM Library. The application prints the camera pose translation to the console and draws the occupancy map in a separate window. You can pass the filename of a RealSense SDK recording as a command line argument, and it will play back the recording instead of using live camera data (see the SLAM Developer's Guide for more information). It also demonstrates how to save the occupancy map as a PPM file. 

This sample is the recommended starting point for learning the SLAM API.

## Running

Attach the ZR300 camera and invoke sample_slam_tutorial_1_gui from a console window. A new map window will open. Move the camera around, and you will see its trajectory and the occupancy map being drawn in the map window. It is best to move the camera slowly and smoothly. 

Experiment with moving the camera at different speeds. You can see the numbers for the camera translation printing in the console, along with the tracking accuracy. The translation units are in meters. The tracking accuracy will start off `low`, and should become `medium` once you've moved the camera around a bit. Currently, there is no `high` accuracy; it is reserved for future use. In the map window, the occupancy map is drawn with blue representing occupied areas, and white representing unoccupied areas. Gray areas are where there is no data, and are unknown.

Press the Escape key in the console window to stop and exit the program. It is important to stop the app this way so that the camera is shut down properly.

### Sample console output

```bash
$ sample_slam_tutorial_1_gui
Starting SLAM...
Press Esc key to exit

Translation:(X=-0.08, Y=-0.38, Z=-0.13) Accuracy:med

Esc key pressed
Stopping...
Saving occupancy map to disk...
```

# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_slam_tutorial_1_gui
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_slam_tutorial_1_gui.



## License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
