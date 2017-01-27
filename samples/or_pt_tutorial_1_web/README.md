# or_pt_tutorial_1_web

This GUI app builds on the or_pt_tutorial_1. It displays the live color preview from the camera within a browser and draw rectangles around the person(s) and object(s) that are detected in the camera frame. There is also a table showing the person ID and center of mass in world coordinates (x,y,z), along with object label, object center of mass, and confidence score.

# Sample output
When running the or_pt_tutorial_1_web sample:

 - The object localization result will be printed on the console with the object's
   label name, recognition confidence value, color, and world coordinates.
 - The person tracking result contains person IDs and their 2D color coordinates.

###Console output

```
Person ID         Person Center               2D Coordinates
---------         -------------               --------------
0                 (0.07779,0.3685,2.96)       (287,160) (348,479)

Person ID         Person Center               2D Coordinates
---------         -------------               --------------
1                 (1.449,0.5508,3.299)        (555,272) (636,434)

Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
person            0.99              (-0.016,0.34,2.8)           (250,158) (371,471)
light             0.94              (0,0,0)                     (3,113) (78,139)
person            0.88              (1.3,0.61,2.9)              (535,264) (635,478)
light             0.87              (-1.4,-1.6,5.9)             (96,45) (245,107)
light             0.85              (-0.35,-0.31,2.9)           (191,160) (288,186)
table             0.71              (1,1,4.7)                   (368,337) (526,409)

```

###Screen shot

![Image](./docs/or_pt_gui_tutorial_1.png?raw=true)

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_pt_tutorial_1_web
```
**Note:** If you are building this sample from source, the executable name is, instead, sample_or_pt_tutorial_1_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.


#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
