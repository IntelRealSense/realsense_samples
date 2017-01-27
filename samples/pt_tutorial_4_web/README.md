

#pt_tutorial_4_web

This GUI app illustrates how to register new users to the database and identify them when they appear in the scene. It displays the live color preview from the camera within a browser and draw rectangles around the person(s) detected in the camera frame with a color dot indicating the center of mass for the detected persons in the frame. It also displays the Person ID (pid) for every person detected which is temporary id for the person. Mouse click in any of the detected person's rectangle, activates registration for that person and saves the information in a database (database name currently hardcoded) with a Recognition ID (rid) assigned to that person. Clicking on the **load database** button, loads the database that has information about the recognized persons and displays the **rid** of already registered peson when they are present in the FOV.  


# Sample output

When running pt_tutorial_4_web sample, the number of persons detected in the current frame will be printed on console along with the cumulative total:

```
Current Frame Total      Cumulative
--------------------     ----------
0                        0

Current Frame Total      Cumulative
--------------------     ----------
1                        1

Web UI: on_data_string: received request to track person 0

save recognition id to database: ./person_recognition_id.db

Current Frame Total      Cumulative
--------------------     ----------
0                        1

Current Frame Total      Cumulative
--------------------     ----------
1                        2

server: received load_db message

loading recognition id database: ./person_recognition_id.db

Registered person: 102

```

Screen shot

![Image](./docs/pt_gui_tutorial_4.png?raw=true)


# Steps to execute:

Samples are ready to run when installed using the debian package. Type the name of the sample in the terminal and run it.

Open the web page to the url link provided on the terminal which is "TargetIPaddr:8000/view.html"; 
e.g. 10.30.90.130:8000/view.html

If you want to modify a sample and run it, refer to the **Building** section in the main README.md.

#License

Copyright 2016 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
