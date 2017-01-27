#pt_tutorial_3_web

This GUI app builds on pt_tutorial_3 and displays the live color preview from the camera within a browser. It draws a rectangle around the person detected in the camera frame. It also draws an arrow to indicate the direction of the pointing gesture. There is also a table as part of the GUI that shows the person ID, gesture origin (x,y,z) and gesture direction.

# Sample output

When running the pt_tutorial_3_web sample and a gesture is detected, a "Pointing Detected" alert, along with pointing origin and direction values are continuously printed on the console.

```
Pointing detected, PID: 0
    color coordinates: origin(x,y): 97, 58
                    direction(x,y): 0.111, 0.15
    world coordinates: origin(x,y,z): -0.238413, -0.282607, 1.436
                    direction(x,y,z): 0.49031, 0.643527, -0.194
Pointing detected, PID: 0
    color coordinates: origin(x,y): 97, 58
                    direction(x,y): 0.111, 0.15
    world coordinates: origin(x,y,z): -0.238413, -0.282607, 1.436
                    direction(x,y,z): 0.49031, 0.643527, -0.194
Pointing detected, PID: 0
    color coordinates: origin(x,y): 97, 58
                    direction(x,y): 0.111, 0.15
    world coordinates: origin(x,y,z): -0.238413, -0.282607, 1.436
                    direction(x,y,z): 0.49031, 0.643527, -0.194
```

Screen shot

![Image](./docs/pt_gui_tutorial_3.png?raw=true)


# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_pt_tutorial_3_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_pt_tutorial_3_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
