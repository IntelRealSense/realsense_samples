// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include "module_manager.h"
#include "version.h"

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

typedef struct running_para
{
    running_para() : slam_enabled(false), or_enabled(false), pt_enabled(false) {};
    void enable_all()
    {
        slam_enabled = true;
        or_enabled = true;
        pt_enabled = true;
    }
    bool slam_enabled;
    bool or_enabled;
    bool pt_enabled;
} running_para;

void parse_args(int argc, char* argv[], running_para& para)
{
    if(argc == 1)
    {
        para.enable_all();
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            std::string arg(argv[i]);
            if(arg == "-slam")
            {
                para.slam_enabled = true;
            }
            else if(arg == "-or")
            {
                para.or_enabled = true;
            }
            else if(arg == "-pt")
            {
                para.pt_enabled = true;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    Module_manager module_manager;

    // Parse parameters
    running_para para;
    parse_args(argc, argv, para);

    // Enable MW
    module_manager.set_slam_enabled(para.slam_enabled);
    module_manager.set_object_recognition_enabled(para.or_enabled);
    module_manager.set_person_tracking_enabled(para.pt_enabled);
    module_manager.config_modules();

    // Initialize camera manager
    if (!Camera_manager::has_camera())
    {
        cout << "Error: There are no RealSense devices connected." << endl << "Please connect a RealSense device and restart the application" << endl;
        return -1;
    }
    Camera_manager camera_manager(&module_manager);
    camera_manager.set_camera_configer((camera_config_interface*)&module_manager);

    // Initialize module manager
    module_manager.init(argv[0]);


    // Config and start camera device
    camera_manager.start_camera();

    // Start sample loop
    module_manager.run();

    return 0;
}
