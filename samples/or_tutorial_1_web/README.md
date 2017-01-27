# or_tutorial_1_web

This GUI app builds the or_tutorial_1 app and displays the live color preview from the camera within a browser and shows a table with a label for the object, along with its confidence value.

# Sample output

When running the or_tutorial_1_web sample, the object recognition result will be printed on the console as well as displayed in a table on the browser continuously with the object's label and recognition confidence value.

###Console output

```
Label             Confidence
-----             ----------
Toys               0.99

Label             Confidence
-----             ----------
Toys               0.99
```

###Screen shot
![Image](./docs/or_gui_tutorial_1.png?raw=true)

# Steps to execute

Type the following command at the command prompt to execute this sample:

```bash
$ rs_or_tutorial_1_web
```

**Note:** If you are building this sample from source, the executable name is, instead, sample_or_tutorial_1_web.

When the sample runs, it starts a built-in web browser and prints the URL of your sample to a console, in the format:   
    
    <target_ip_add>:8000>/view.html

For example:  10.30.90.130:8000/view.html

You can launch the web browser of your choice locally or remotely by navigating to the IP address displayed in the console.


#License

Copyright 2017 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this project except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
