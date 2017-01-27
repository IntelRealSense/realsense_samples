// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <thread>
#include <librealsense/rs.hpp>
#include "rs_sdk.h"
#include <iostream>
#include "version.h"

#include "or_utils.hpp"
#include "or_console_display.hpp"
#include "or_web_display.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

// Doing the OR processing for a frame can take longer than the frame interval, so we
// keep track of whether or not we are still processing the last frame.
bool is_or_processing_frame = false;
bool is_exit = false;

unique_ptr<web_display::or_web_display>  web_view;
unique_ptr<console_display::or_console_display>    console_view;

// Use a queue to hold the sample set waiting for processing
blocking_queue<correlated_sample_set*> sample_set_queue;

// Run object localization and sending result to view
void run_object_localization(or_video_module_impl* impl, or_data_interface* or_data,
                             or_configuration_interface* or_configuration)
{
    rs::core::status status;

    //Declare data structure and size for results
    rs::object_recognition::localization_data* localization_data = nullptr;
    int array_size = 0;

    correlated_sample_set* or_sample_set = nullptr;
    while(!is_exit && (or_sample_set = sample_set_queue.pop()) != NULL)
    {

        //Run object localization processing
        status = impl->process_sample_set(*or_sample_set);

        // Recycle sample set after processing complete
        if((*or_sample_set)[stream_type::color])
            (*or_sample_set)[stream_type::color]->release();
        if((*or_sample_set)[stream_type::depth])
            (*or_sample_set)[stream_type::depth]->release();
        (*or_sample_set)[stream_type::color] = nullptr;
        (*or_sample_set)[stream_type::depth] = nullptr;

        if (status != rs::core::status_no_error)
        {
            is_or_processing_frame = false;
            return;
        }

        //Retrieve recognition data from the or_data object
        status = or_data->query_localization_result(&localization_data, array_size);
        if (status != rs::core::status_no_error)
        {
            is_or_processing_frame = false;
            return;
        }

        //Send or data to ui
        if (localization_data)
        {
            web_view->on_object_localization_data(localization_data, array_size, or_configuration);
            if (array_size != 0)
            {
                console_view->on_object_localization_data(localization_data, array_size, or_configuration);
            }
        }

        is_or_processing_frame = false;
    }
}


int main(int argc,char* argv[])
{
    rs::core::status status;

    // Initialize camera and OR module
    or_utils                or_util;
    rs::core::image_info        colorInfo, depthInfo;
    or_video_module_impl        impl;
    or_data_interface           *or_data = nullptr;
    or_configuration_interface  *or_configuration = nullptr;

    or_util.init_camera(colorInfo, depthInfo, impl, &or_data, &or_configuration);
    int imageWidth = or_util.get_color_width();
    int imageHeight = or_util.get_color_height();
    string sample_name = argv[0];

    // Change mode to localization
    or_configuration->set_recognition_mode(recognition_mode::LOCALIZATION);

    // Set the localization mechanism to use CNN
    or_configuration->set_localization_mechanism(localization_mechanism::CNN);

    // Ignore all objects under 0.7 probability (confidence)
    or_configuration->set_recognition_confidence(0.7);

    // Enabling object center feature
    or_configuration->enable_object_center_estimation(true);

    status = or_configuration->apply_changes();
    if (status != rs::core::status_no_error)
        return status;

    // Create and start remote(Web) view
    web_view = move(web_display::make_or_web_display(sample_name, 8000, true));
    // Create console view
    console_view = move(console_display::make_console_or_display());

    vector<string> obj_name_list;
    or_util.query_object_name_list(obj_name_list, or_configuration);
    web_view->on_object_list(obj_name_list);
    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    // Start background thread to run recognition processing
    std::thread recognition_thread(run_object_localization,
                                   &impl, or_data, or_configuration);
    recognition_thread.detach();

    while (!(is_exit = or_util.user_request_exit()))
    {
        correlated_sample_set* sample_set = or_util.get_sample_set(colorInfo, depthInfo);

        if (!is_or_processing_frame)
        {
            is_or_processing_frame = true;

            // Increase image reference to hold for library processing
            (*sample_set)[rs::core::stream_type::color]->add_ref();
            (*sample_set)[rs::core::stream_type::depth]->add_ref();
            // Push the sample set to queue
            sample_set_queue.push(sample_set);
        }

        //Display color image
        auto colorImage = (*sample_set)[rs::core::stream_type::color];

        // Sending dummy time stamp of 10.
        web_view->on_rgb_frame(10, imageWidth, imageHeight, colorImage->query_data());
    }

    // Stop the camera
    or_util.stop_camera();
    cout << "-------- Stopping --------" << endl;

    return 0;
}
