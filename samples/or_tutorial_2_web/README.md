# or_tutorial_2_web

This GUI app builds on the or_tutorial_2 app and displays the live color preview from the camera within a browser and draws rectangles on the color images in the region where objects are recognized. This GUI also includes a table that shows a label for the object along with its confidence value and the 3D(x,y,z) coordinates of the object. In theis tutorial, all items with >= 90% confidence are bound by a green colored rectangle. Based on the confidence levels, the bounding rectangle color will change (green >= 90%, red <=50%, yellow for the rest).

# Sample output

When running the or_tutorial_2_web sample, the object localization result is printed on the console, similarly to the or_tutorial_2 app, with object label name, recognition confidence value, center and coordinates (see below). The same information will be presented in a table in the browser.

###Console output
```
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
cup               0.98              (-0.047,0.11,0.6)           (201,304) (331,412)
table             0.8               (-0.0036,0.15,0.55)         (0,349) (620,478)

Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
cup               0.99              (-0.031,0.096,0.61)         (217,278) (349,397)
table             0.88              (0.0048,0.14,0.54)          (0,332) (639,466)

```

###Screen shot 
![Image](./docs/or_gui_tutorial_2.png?raw=true)

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_tutorial_2_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_2_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
