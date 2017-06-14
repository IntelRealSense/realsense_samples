// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <thread>
#include <iostream>
#include <signal.h>
#include "version.h"
#include "pt_utils.hpp"
#include "pt_console_display.hpp"
#include "or_console_display.hpp"


using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

// Doing the OR processing for a frame can take longer than the frame interval, so we
// keep track of whether or not we are still processing the last frame.
bool is_or_processing_frame = false;

bool play = true;

unique_ptr<console_display::pt_console_display> pt_console_view = move(console_display::make_console_pt_display());
unique_ptr<console_display::or_console_display> or_console_view = move(console_display::make_console_or_display());

void processing_OR(correlated_sample_set or_sample_set, or_video_module_impl* impl,
                   or_data_interface* or_data, or_configuration_interface* or_configuration)
{
    rs::core::status st;

    //Declare data structure and size for results
    rs::object_recognition::localization_data* localization_data = nullptr;

    //Run object localization or tracking processing
    st = impl->process_sample_set(or_sample_set);
    if (st != rs::core::status_no_error)
    {
        is_or_processing_frame = false;
        return;
    }

    // Retrieve recognition data from the or_data object
    int array_size = 0;
    st = or_data->query_localization_result(&localization_data, array_size);
    if (st != rs::core::status_no_error)
    {
        is_or_processing_frame = false;
        return;
    }

    // rint OR result on console
    if (localization_data && array_size != 0)
    {
        or_console_view->on_object_localization_data(localization_data, array_size,or_configuration);
    }

    is_or_processing_frame = false;
}


int main(int argc,char* argv[])
{
    rs::core::status st;
    pt_utils pt_utils;

    rs::core::image_info colorInfo,depthInfo;
    rs::core::video_module_interface::actual_module_config actualModuleConfig;
    rs::person_tracking::person_tracking_video_module_interface* ptModule = nullptr;
    rs::object_recognition::or_video_module_impl impl;
    rs::object_recognition::or_data_interface* or_data = nullptr;
    rs::object_recognition::or_configuration_interface* or_configuration = nullptr;

    //Init camera, object recognition and person tracking modules
    if(pt_utils.init_camera(colorInfo,depthInfo,actualModuleConfig,impl,&or_data,&or_configuration) != rs::core::status_no_error)
    {
        cerr << "Error: Device is null."<< endl << "Please connect a RealSense device and restart the application" << endl;
        return -1;
    }
    pt_utils.init_person_tracking(&ptModule);

    //Person Tracking Configuration. Set tracking mode to 0
    ptModule->QueryConfiguration()->QueryTracking()->Enable();
    ptModule->QueryConfiguration()->QueryTracking()->SetTrackingMode((Intel::RealSense::PersonTracking::PersonTrackingConfiguration::TrackingConfiguration::TrackingMode)0);
    if(ptModule->set_module_config(actualModuleConfig) != rs::core::status_no_error)
    {
        cerr<<"error : failed to set the enabled module configuration" << endl;
        return -1;
    }

    //Object Recognition Configuration
    //Set mode to localization
    or_configuration->set_recognition_mode(rs::object_recognition::recognition_mode::LOCALIZATION);
    //Set the localization mechnizm to use CNN
    or_configuration->set_localization_mechanism(rs::object_recognition::localization_mechanism::CNN);
    //Ignore all objects under 0.7 probabilty (confidence)
    or_configuration->set_recognition_confidence(0.7);
    //Enabling object center feature
    or_configuration->enable_object_center_estimation(true);

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return st;

    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    while (!pt_utils.user_request_exit())
    {
        //Get next frame
        rs::core::correlated_sample_set* sample_set = pt_utils.get_sample_set(colorInfo,depthInfo);
        rs::core::correlated_sample_set* sample_set_pt = pt_utils.get_sample_set(colorInfo,depthInfo);

        //Increment reference count of images at sample set
        for (int i = 0; i < static_cast<uint8_t>(rs::core::stream_type::max); ++i)
        {
            if (sample_set_pt->images[i] != nullptr)
            {
                sample_set_pt->images[i]->add_ref();
            }
        }


        //Run object recognition in a separate thread. Print the result
        if (!is_or_processing_frame)         // If we aren't already processing or for a frame:
        {
            is_or_processing_frame = true;
            std::thread recognition_thread(processing_OR, *sample_set,
                                           &impl, or_data, or_configuration);
            recognition_thread.detach();
        }

        //Run Person Tracking
        if (ptModule->process_sample_set(*sample_set_pt) != rs::core::status_no_error)
        {
            cerr << "error : failed to process sample" << endl;
            continue;
        }

        //Print person tracking result
        pt_console_view->on_person_info_update(ptModule);
    }

    pt_utils.stop_camera();
    actualModuleConfig.projection->release();

    return 0;
}
