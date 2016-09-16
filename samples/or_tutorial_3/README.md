# or_tutorial_3

This console app builds on top of or_tutorial_2, and illustrates how to add the libOR object tracking functionality with localization and 3d localization. Objects for tracking can be as small as 1% of the screen

# Sample output

When running or_tuturial_3 sample, the object localization and tracking result will be printed  on console continuously.

First localization result comes out with objects' label name, recognition confidence value, center and coordinates, and then sample will track these localized objects and print out their label and new coordinates:


```
localization object information:
Label             Confidence        Object Center               Coordinate
-----             ----------        ---------------             ------------
table             0.88              (0,0,0)                     (0,303) (1435,1039)
monitor           0.79              (0,0,0)                     (399,11) (1222,598)
chair             0.78              (0,0,0)                     (998,372) (1635,1042)

Will track all of these objects.

Label             Coordinate
-----             ------------
table             (0,303) (1435,1039)
monitor           (399,11) (1222,598)
chair             (998,372) (1635,1042)

Label             Coordinate
-----             ------------
table             (0,257) (1435,993)
monitor           (399,-26) (1222,561)
chair             (998,352) (1635,1022)
```

#License

Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
