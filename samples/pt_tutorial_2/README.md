#pt_tutorial_2

This sample app demonstrates how to analyze someone’s posture. When a person is in the field of view (FOV), the app should display the following information once every second: head direction (yaw, pitch, roll values).

# Sample output

When running the pt_tutorial_2 sample, person ID (PID), and head pose values (yaw, pitch, roll) will be printed on the console continuously.

```
PID: 0, head pose (pitch, roll, yaw) (-7.50198, -11.7497, 1.65109)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-6.33085, -11.9885, 3.56357)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-7.16278, -12.8903, 1.13986)
 orientation: frontal, confidence: 100
PID: 0, head pose (pitch, roll, yaw) (-7.34624, -14.0557, -0.528061)
```

# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_pt_tutorial_2
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_pt_tutorial_2.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
