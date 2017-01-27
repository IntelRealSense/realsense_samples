# or_pt_tutorial_1

This console app illustrates the use of Intel® RealSense™ Cross Platform API, RealSense SDK for Linux Object and Person libraries using the ZR300 camera identify objects and persons. The name of the object along with the confidence value will be printed on the console. Person information like person ID and the 2D box coordinates will be printed on the console.

# Sample output
When running the or_pt_tutorial_1 sample:

 - The object localization result will be printed on the console with object's label name, recognition confidence value, color, and world coordinates.
 - The person tracking result contains person IDs and their 2D color coordinates.


```
Person ID         Person Center               2D Coordinates
---------         -------------               --------------
0                 (-0.1505,0.07124,0.916)     (53,92) (292,479)

Label             Object Center               2D Coordinates          Confidence
-----             --------------              ---------------         ----------
chair             (381.1,539,2236)            (344,295) (509,478)     97.1
pillow            (-918.2,-277.4,2412)        (5,55) (171,280)        79.2
pillow            (-199.4,-98.22,898.4)       (103,44) (268,298)      73.8
light             (730.1,115.2,2422)          (492,241) (522,294)     71.15

Label             Object Center               2D Coordinates          Confidence
-----             --------------              ---------------         ----------
chair             (402.9,557.7,2354)          (345,291) (509,477)     95.51

Label             Object Center               2D Coordinates          Confidence
-----             --------------              ---------------         ----------
chair             (380.3,537.4,2275)          (333,293) (516,474)     94.55

Label             Object Center               2D Coordinates          Confidence
-----             --------------              ---------------         ----------
chair             (402.9,542.7,2236)          (344,297) (521,478)     95.18

Person ID         Person Center               2D Coordinates
---------         -------------               --------------
0                 (-0.1132,0.08556,0.9252)    (79,126) (333,478)

```

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_pt_tutorial_1
```
**Note:** If you are building this sample from source, the executable name is, instead, sample_or_pt_tutorial_1.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
