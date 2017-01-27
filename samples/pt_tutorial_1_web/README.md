#pt_tutorial_1_web

This GUI app builds on pt_tutorial_1 and displays the live color preview from the camera within a browser. It draws rectangles around the person(s) detected in the camera frame and draws a colored dot indicating the center of mass for the detected person in the image. There is also a table as part of the GUI that shows the person ID, center of mass world coordinates (x,y,z) and the cumulative count of persons detected.


# Sample output

When running the pt_tutorial_1_web sample, the number of persons detected in the current frame will be printed on console along with the cumulative total.

####Sample output
```
Current Frame Total      Cumulative
--------------------     ----------
0                        0

Current Frame Total      Cumulative
--------------------     ----------
1                        1
```
####Screen shot

![Image](./docs/pt_gui_tutorial_1.png?raw=true)


# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_pt_tutorial_1_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_pt_tutorial_1_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
