// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <thread>
#include <iostream>
#include "version.h"
#include "or_utils.hpp"
#include "or_console_display.hpp"
#include "or_gui_display.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

// Doing the OR processing for a frame can take longer than the frame interval, so we
// keep track of whether or not we are still processing the last frame.
bool is_or_processing_frame = false;
bool is_exit = false;

unique_ptr<console_display::or_console_display>    console_view;
unique_ptr<gui_display::or_gui_display> gui_view;

// Run object recognition and sending result to view
void run_object_recognition(rs::core::correlated_sample_set* or_sample_set, or_video_module_impl* impl, or_data_interface* or_data,
                            or_configuration_interface* or_configuration)
{
    rs::core::status st;

    // Declare data structure and size for results
    rs::object_recognition::recognition_data* recognition_data = nullptr;
    int array_size = 0;


    gui_view->set_color_image((*or_sample_set)[rs::core::stream_type::color]);

    // Run object recognition processing
    st = impl->process_sample_set(*or_sample_set);

    // Recycle sample set after processing complete
    if((*or_sample_set)[stream_type::color])
        (*or_sample_set)[stream_type::color]->release();
    if((*or_sample_set)[stream_type::depth])
        (*or_sample_set)[stream_type::depth]->release();

    (*or_sample_set)[stream_type::color] = nullptr;
    (*or_sample_set)[stream_type::depth] = nullptr;

    if (st != rs::core::status_no_error)
    {
        is_or_processing_frame = false;
        return;
    }

    // Retrieve recognition data from the or_data object
    st = or_data->query_single_recognition_result(&recognition_data, array_size);
    if (st != rs::core::status_no_error)
    {
        is_or_processing_frame = false;
        return;
    }

    // Print out the recognition results
    if (recognition_data)
    {
        console_view->on_object_recognition_data(recognition_data, array_size, or_configuration);

        gui_view->draw_results(recognition_data, array_size, or_configuration);
    }

    is_or_processing_frame = false;

}


int main(int argc,char* argv[])
{
    rs::core::status st;
    or_utils or_utils;

    cout << "Initializing, one moment please ..." << endl << endl;

    console_view = move(console_display::make_console_or_display());
    gui_view = move(gui_display::make_gui_or_display());

    rs::core::image_info colorInfo,depthInfo;
    or_video_module_impl impl;
    or_data_interface* or_data = nullptr;
    or_configuration_interface* or_configuration = nullptr;

    // Initializing Camera and Object Recognition modules
    or_utils.init_camera(colorInfo,depthInfo, impl, &or_data, &or_configuration,true);

    // Configure Object Recognition to recognize objects which take up ~50% of the screen
    int width = colorInfo.width;
    int height = colorInfo.height;
    or_configuration->set_roi(rs::core::rect{(width/4), (height/4), (width/2), (height/2)});
    // Enable GPU computing
    or_configuration->set_compute_engine(rs::object_recognition::compute_engine::GPU); 

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return st;

    gui_view->initialize(colorInfo,"Recognition");

    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    while (!(is_exit = or_utils.user_request_exit()))
    {
        rs::core::correlated_sample_set* sample_set = or_utils.get_sample_set(colorInfo,depthInfo);

        // Recognition is not a real-time process, so it is not required to run every frame
        if(!is_or_processing_frame)
        {
            is_or_processing_frame = true;

            // Increase image reference to hold for library processing
            (*sample_set)[rs::core::stream_type::color]->add_ref();
            (*sample_set)[rs::core::stream_type::depth]->add_ref();
            run_object_recognition(sample_set, &impl, or_data, or_configuration);
        }
        gui_view->show_results();

    }

    or_utils.stop_camera();
    cout << "-------- Stopping --------" << endl;

    return 0;
}
