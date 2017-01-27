#pt_tutorial_3

This sample app demonstrates how to get the body tracking points and detect a pointing gesture. The app will display six body points, a  “Pointing Detected” alert when the gesture is performed, and the pointing vector.

# Sample output

When running the pt_tutorial_3 sample and a gesture is detected, a "Pointing Detected" alert and pointing origin and direction values are printed continuously on the console:

```
Pointing detected, PID: 0
    color coordinates: origin(x,y): 250, 141
                    direction(x,y): 0.2, 0.324
    world coordinates: origin(x,y,z): -0.100094, -0.213276, 1.356
                    direction(x,y,z): 0.421522, 0.678274, -0.098
Pointing detected, PID: 0
    color coordinates: origin(x,y): 250, 141
                    direction(x,y): 0.2, 0.324
    world coordinates: origin(x,y,z): -0.100094, -0.213276, 1.356
                    direction(x,y,z): 0.421522, 0.678274, -0.098
```

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_pt_tutorial_3
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_pt_tutorial_3.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
