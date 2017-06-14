// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <thread>
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

// Run object localization and sending result to view
void run_object_localization(correlated_sample_set* or_sample_set, or_video_module_impl* impl, or_data_interface* or_data,
                             or_configuration_interface* or_configuration)
{
    rs::core::status st;

    // Declare data structure and size for results
    rs::object_recognition::localization_data* localization_data = nullptr;
    int array_size = 0;

     // Run object localization processing
        st = impl->process_sample_set(*or_sample_set);
        gui_view->set_color_image((*or_sample_set)[stream_type::color]);


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
    st = or_data->query_localization_result(&localization_data, array_size);
    if (st != rs::core::status_no_error)
    {
        is_or_processing_frame = false;
        return;
    }

    // Print localization result on console
    if (localization_data && array_size != 0)
    {
        console_view->on_object_localization_data(localization_data, array_size, or_configuration);
        gui_view->draw_results(localization_data, array_size, or_configuration);
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

    // Init camera and OR module
    or_utils.init_camera(colorInfo,depthInfo,impl,&or_data,&or_configuration,true);

    // Change mode to localization
    or_configuration->set_recognition_mode(recognition_mode::LOCALIZATION);
    // Set the localization mechanism to use CNN
    or_configuration->set_localization_mechanism(localization_mechanism::CNN);
    // Ignore all objects under 0.7 probability (confidence)
    or_configuration->set_recognition_confidence(0.7);
    // Enabling object center feature
    or_configuration->enable_object_center_estimation(true);
    // Enable GPU computing
    or_configuration->set_compute_engine(rs::object_recognition::compute_engine::GPU); 

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return st;

    gui_view->initialize(colorInfo,"Localization");

    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    while (!(is_exit = or_utils.user_request_exit()))
    {
        correlated_sample_set* sample_set = or_utils.get_sample_set(colorInfo,depthInfo);

        // Display color image
        auto colorImage = (*sample_set)[rs::core::stream_type::color];
        console_view->render_color_frames(colorImage);

        if(!is_or_processing_frame)
        {
            is_or_processing_frame = true;

            // Increase image reference to hold for library processing
            (*sample_set)[rs::core::stream_type::color]->add_ref();
            (*sample_set)[rs::core::stream_type::depth]->add_ref();
            run_object_localization(sample_set, &impl, or_data, or_configuration);

        }

        gui_view->show_results();
    }

    // Close the camera
    or_utils.stop_camera();
    cout << "-------- Stopping --------" << endl;

    return 0;
}
