# or_tutorial_1

This console app illustrates the use of libRealSense, libOR, and the Linux SDK Framework to use the RealSense camera's depth and color sensors to identify objects in the scene. Each object identified will be printed on the command line along with the confidence value for the object, for objects which take up ~50% of the screen for recognition

# Sample output


When running or_tuturial_1 sample, the object recognition result will be printed  on console continuously with objects' label and recognition confidence value:

```
Label             Confidence
-----             ----------
Table              0.91

Label             Confidence
-----             ----------
Monitor            0.86
```

#License

Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
