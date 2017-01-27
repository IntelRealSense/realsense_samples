#pt_tutorial_2_web

This GUI app builds on pt_tutorial_2 and displays the live color preview from the camera within a browser. It draws a rectangle around the head of a person detected in the camera frame. There is also a table as part of the GUI that shows the person ID, and head pose information. 

# Sample output

When running pt_tutorial_2_web sample, person ID (PID), and head pose values (yaw, pitch, roll) will be printed on the console continuously:

###Console output
```
PID: 0, head pose (pitch, roll, yaw) (-7.50198, -11.7497, 1.65109)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-6.33085, -11.9885, 3.56357)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-7.16278, -12.8903, 1.13986)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-7.34624, -14.0557, -0.528061)
```

###Screen shot

![Image](./docs/pt_gui_tutorial_2.png?raw=true)


# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_pt_tutorial_2
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_pt_tutorial_2.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
