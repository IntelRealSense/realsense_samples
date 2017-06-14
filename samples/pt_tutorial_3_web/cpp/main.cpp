// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <signal.h>

#include <librealsense/rs.hpp>
#include "rs_sdk.h"
#include "version.h"
#include "pt_utils.hpp"
#include "pt_console_display.hpp"
#include "pt_web_display.hpp"

using namespace std;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

unique_ptr<web_display::pt_web_display>  web_view;



void set_tracking(rs::person_tracking::person_tracking_video_module_interface* ptModule)
{
    Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

    if (trackingData->GetTrackingState() == Intel::RealSense::PersonTracking::PersonTrackingData::TrackingState::TRACKING_STATE_DETECTING &&
            trackingData->QueryNumberOfPeople() > 0)
    {
        // Start tracking the first person detected in the frame
        Intel::RealSense::PersonTracking::PersonTrackingData::Person* personData = trackingData->QueryPersonData(Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);
        if (personData)
        {
            cout << "Call StartTracking()" <<endl;
            trackingData->StartTracking(personData->QueryTracking()->QueryId());
        }
    }
}

int main(int argc, char** argv)
{

    pt_utils pt_utils;

    rs::core::video_module_interface::actual_module_config actualModuleConfig;
    rs::person_tracking::person_tracking_video_module_interface* ptModule = nullptr;


    // Initializing Camera and Person Tracking modules
    if(pt_utils.init_camera(actualModuleConfig) != rs::core::status_no_error)
    {
        cerr << "Error: Device is null." << endl << "Please connect a RealSense device and restart the application" << endl;
        return -1;
    }
    pt_utils.init_person_tracking(&ptModule);

    // Enable Pointing Gesture
    ptModule->QueryConfiguration()->QueryGestures()->Enable();
    ptModule->QueryConfiguration()->QueryGestures()->EnableAllGestures();
    ptModule->QueryConfiguration()->QueryTracking()->Enable();

    // Configure enabled Pointing Gesture
    if(ptModule->set_module_config(actualModuleConfig) != rs::core::status_no_error)
    {
        cerr<<"Error : Failed to configure the enabled Pointing Gesture" << endl;
        return -1;
    }

    // Start the camera
    pt_utils.start_camera();

    // Launch GUI
    string sample_name = argv[0];
    // Create and start remote(Web) view
    web_view = move(web_display::make_pt_web_display(sample_name, 8000, true));

    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    while(!pt_utils.user_request_exit())
    {
        rs::core::correlated_sample_set sampleSet = {};

        // Get next frame
        if (pt_utils.GetNextFrame(sampleSet) != 0)
        {
            cerr << "Error: Invalid frame" << endl;
            continue;
        }

        // Process frame
        if (ptModule->process_sample_set(sampleSet) != rs::core::status_no_error)
        {
            cerr << "Error : Failed to process sample" << endl;
            continue;
        }

        auto colorImage = sampleSet[rs::core::stream_type::color];

        // Sending dummy timestamp of 10
        // Display GUI, pointing gesture info
        web_view->on_rgb_frame(10, colorImage->query_info().width, colorImage->query_info().height, colorImage->query_data());

        // Start tracking the first person detected in the frame
        set_tracking(ptModule);

        web_view->on_pt_pointing_gesture_update(ptModule);

        // Release color and depth image
        sampleSet.images[static_cast<uint8_t>(rs::core::stream_type::color)]->release();
        sampleSet.images[static_cast<uint8_t>(rs::core::stream_type::depth)]->release();

    }

    pt_utils.stop_camera();
    actualModuleConfig.projection->release();
    cout << "-------- Stopping --------" << endl;
    return 0;
}
