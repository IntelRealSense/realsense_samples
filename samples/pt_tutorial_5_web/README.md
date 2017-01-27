

#pt_tutorial_5_web

This GUI app builds on top of pt_tutorial_5 and displays the live color preview from the camera within a browser and draw rectangles around the person(s) detected in the camera frame and draws a color dot indicating the center of mass for the detected person in the image. Mouse click in any of the rectangle activates tracking for that person. To stop tracking, click outside the rectangle of the person who is being tracked. There is also a table as part of the GUI that shows the person id, center of mass world coordinates(x,y,z) and the cumulative count of persons detected. When the person moves out of frame, tracking on the person works for a few seconds. Once the tracking module loses track of the person it was tracking, it goes back to the person detection mode. 


# Sample output

When running pt_tutorial_5_web sample, the number of persons detected in the current frame will be printed on console along with the cumulative total:

```
Current Frame Total      Cumulative
--------------------     ----------
0                        0

Current Frame Total      Cumulative
--------------------     ----------
1                        1
```
Screen shot

![Image](./docs/pt_gui_tutorial_1.png?raw=true)


# Steps to execute:

Samples are ready to run when installed using the debian package. Type the name of the sample in the terminal and run it.

Open the web page to the url link provided on the terminal which is "TargetIPaddr:8000/view.html"; 
e.g. 10.30.90.130:8000/view.html

If you want to modify a sample and run it, refer to the **Building** section in the main README.md.

#License

Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
