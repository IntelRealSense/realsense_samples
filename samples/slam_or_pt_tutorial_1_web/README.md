# slam_or_pt_tutorial_1_web

This GUI app builds on slam_or_pt_tutorial_1 and displays live fisheye and color preview, occupancy map, input, and tracking FPS for fisheye, depth, gyro, and accelerometer frames within a browser. Draws rectangles around recognized objects and persons in the color preview. 

# Sample output

You can see the SLAM, object recognition, and person tracking results printed on the console.

###Console output

```
error: Junk time stamp in stream:4	with frame counter:100
server: got connection 127.0.0.1:59272
Error: Required key Tc0 not found
Error: Required key Wc0 not found
creating occupancy map: resolution: 0.05
OR init complete
init_pt complete
Press Enter key to exit
Current Frame Total      Cumulative
--------------------     ----------
0                        0

camera pose for Object Recognition frame:( 0.00,  0.02, -0.05)
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
pillow            0.91              (1.2e+02,-65,5.4e+02)       (346,5) (548,324)
light             0.8               (-2.7e+03,-1.9e+03,7.1e+03) (12,40) (124,106)
light             0.74              (-5.2e+02,-1.8e+03,5e+03)   (167,0) (318,44)
pillow            0.72              (2.1e+02,-28,5.2e+02)       (511,160) (591,252)
light             0.71              (0,0,0)                     (188,168) (249,193)

occ. map: tiles updated=964	tile_0: pos=(-0.05,  0.50) occupancy=100%
occ. map: tiles updated=165	tile_0: pos=(-0.15,  0.55) occupancy= 9%
occ. map: tiles updated=169	tile_0: pos=(-0.20,  0.90) occupancy= 1%
occ. map: tiles updated=94	tile_0: pos=(-0.20,  0.90) occupancy= 0%
occ. map: tiles updated=114	tile_0: pos=(-0.20,  0.90) occupancy= 0%
occ. map: tiles updated=213	tile_0: pos=(-0.20,  0.90) occupancy= 0%
Current Frame Total      Cumulative
--------------------     ----------
1                        1
```

###Screen shot

![Image](./docs/slam_or_pt_gui_tutorial_1.png?raw=true)


# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_slam_or_pt_tutorial_1_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_slam_or_pt_tutorial_1_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.

## License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
