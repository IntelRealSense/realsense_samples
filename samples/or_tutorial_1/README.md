# or_tutorial_1

This console app illustrates the use of Intel® RealSense™ Cross Platform API, Object Library, and the Linux SDK Framework to use the ZR300 camera's depth and color sensors to identify objects in the scene. Each object identified will be printed on the command line, and (for recognition) the confidence value for the object, for objects which take up ~50% of the screen.

# Sample output


When running or_tutorial_1 sample, the object recognition result will be printed on the console continuously with the object's label and recognition confidence value:

```
Label             Confidence
-----             ----------
Table              0.91

Label             Confidence
-----             ----------
Monitor            0.86
```

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_tutorial_1
```
**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_1.


#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
