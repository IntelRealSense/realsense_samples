# or_tutorial_2

This console app builds on top of or_tutorial_1, and illustrates how to utilize the libOR object localization and 3d localization functionality. All items with >= 90% confidence will identified along with the bounding box coordinates being displayed on the console

# Sample output


When running or_tuturial_2 sample, the object localization result will be printed  on console continuously with objects' label name, recognition confidence value, center and coordinates:


```
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
monitor           0.93              (1.4e+02,-54,6.3e+02)       (768,7) (1837,870)
sink              0.88              (3.3e+02,2.1e+02,7.3e+02)   (1346,854) (1905,1061)

Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
monitor           0.93              (-53,-49,6.6e+02)           (145,2) (1593,909)
picture           0.83              (93,-32,7.3e+02)            (988,192) (1333,803)
```


#License

Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
