// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <signal.h>

#include <librealsense/rs.hpp>
#include "rs_sdk.h"

#include "camera_utils.h"
#include "console_view.h"

using namespace std;

bool play = true;
void my_handler(int s)
{
    if (play)
    {
        play = false;
    }
    else
    {
        exit(1);
    }
}

int main(int argc, char** argv)
{
    // handle ctrl-c
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    camera_utils cu;
    Console_View console_view;

    rs::core::video_module_interface::actual_module_config actualModuleConfig;
    rs::person_tracking::person_tracking_video_module_interface* ptModule = nullptr;

    //Init camera and person tracking modules
    if(cu.init_camera(actualModuleConfig) != rs::core::status_no_error)
    {
        cerr<<"error : device is null" << endl;
        return -1;
    }
    cu.init_person_tracking(&ptModule);

    cout << "Enabling Person head pose and orientation" <<endl;
    ptModule->QueryConfiguration()->QueryTracking()->Enable();
    ptModule->QueryConfiguration()->QueryTracking()->SetTrackingMode((Intel::RealSense::PersonTracking::PersonTrackingConfiguration::TrackingConfiguration::TrackingMode)1);
    ptModule->QueryConfiguration()->QueryTracking()->EnableHeadPose();
    ptModule->QueryConfiguration()->QueryTracking()->EnablePersonOrientation();

    // set the enabled module configuration
    if(ptModule->set_module_config(actualModuleConfig) != rs::core::status_no_error)
    {
        cerr<<"error : failed to set the enabled module configuration" << endl;
        return -1;
    }

    rs::core::correlated_sample_set sampleSet;
    while(play)
    {
        //Get next frame
        if (cu.GetNextFrame(sampleSet) != 0)
        {
            cerr << "error: invalid frame" << endl;
            continue;
        }

        //process frame
        if (ptModule->process_sample_set_sync(&sampleSet) != rs::core::status_no_error)
        {
            cerr << "error : failed to process sample" << endl;
            continue;
        }

        //Print numer of persons in the current frame as well as cumulative total
        console_view.print_headpose_person_orientation_info(ptModule);
    }

    cu.stop_camera();
    cout << "exiting" << endl;
}

