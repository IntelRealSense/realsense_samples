# or_tutorial_2

This console app builds on the or_tutorial_1 app and illustrates how to utilize the object localization and 3D localization functionality. All items with >= 90% confidence will be identified along with the bounding box coordinates being displayed on the console.

# Sample output

When running the or_tutorial_2 sample, the object localization result will be printed on the console continuously with the object's label name, recognition confidence value, center, and coordinates:


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

# Steps to execute

Type the following command at the command prompt to execute this sample:


```bash
$ rs_or_tutorial_2
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_2.

#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
