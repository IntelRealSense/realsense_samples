# slam_tutorial_1_web

This GUI app builds on slam_tutorial_1 and displays live fisheye preview, occupancy map, input, and tracking FPS for fisheye, depth, gyro, and accelerometer frames within a browser.

# Sample output
###Screen shot
[![Image](./docs/slam_gui_tutorial_1.png?raw=true)](./docs/screenvid.mp4?raw=true)

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_slam_tutorial_1_gui
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_slam_tutorial_1_gui.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

## License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
