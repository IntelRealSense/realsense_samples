# or_tutorial_3_web

This GUI app builds on or_tutorial_3 and displays the live color preview from the camera within a browser and draws rectangles on the color images in the region where objects are recognized and keeps track of the objects. This GUI also includes a table that shows a label for the object along with its confidence value and the 2D(x,y) coordinates of the object.

# Sample output

When running the or_tutorial_3_web sample, the object localization and tracking result will be printed on the console continuously, along with the frames being displayed on the browser. A table of recognized objects along with their location will be displayed in a table in the browser.

First, the localization result is displayed with object label name, recognition confidence value, center and coordinates. Then the sample will track these localized objects and print out their label and new coordinates

###Console output

```
localization object information:
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
toys              0.99              (0,0,0)                     (355,168) (451,301)
table             0.96              (0,0,0)                     (0,199) (601,473)
monitor           0.92              (0,0,0)                     (15,141) (253,318)
laptop            0.84              (0,0,0)                     (33,180) (357,375)

Will track all of these objects.

Label             Coordinate
-----             ------------
toys              (355,168) (451,301)
table             (0,199) (601,473)
monitor           (15,141) (253,318)
laptop            (33,180) (357,375)


Label             Coordinate
-----             ------------
toys              (355,168) (451,301)
table             (0,199) (601,473)
monitor           (9,141) (247,318)
laptop            (27,180) (351,375)

```

###Screen shot

![Image](./docs/or_gui_tutorial_3.png?raw=true)

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_tutorial_3_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_3_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
