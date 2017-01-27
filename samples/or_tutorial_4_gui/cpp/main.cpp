// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

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

std::vector<std::string> objects_names;
image_info colorInfo, depthInfo;

bool drag = false;
bool select_flag = false;
bool startTracking = false;
cv::Point point1, point2;

// Doing the OR processing for a frame can take longer than the frame interval, so we
// keep track of whether or not we are still processing the last frame.
bool is_or_processing_frame = false;
bool is_exit = false;

unique_ptr<console_display::or_console_display>    console_view;
unique_ptr<gui_display::or_gui_display> gui_view;

// Use a queue to hold the sample set waiting for processing
blocking_queue<correlated_sample_set*> sample_set_queue;

void mouseHandler(int event, int x, int y, int flags, void* param)
{

    if (event == CV_EVENT_LBUTTONDOWN && !drag && !select_flag)
    {
        // left button clicked. ROI selection begins
        point1 = cv::Point(x, y);
        drag = true;
    }

    if (event == CV_EVENT_MOUSEMOVE && drag && !select_flag)
    {
        // mouse dragged. ROI being selected
        point2 = cv::Point(x, y);
        //calculate the current roi
        rs::core::rect roi;
        roi.x = std::min(point1.x,point2.x);
        roi.y = std::min(point1.y,point2.y);
        roi.width = std::abs(point1.x-point2.x);
        roi.height = std::abs(point1.y-point2.y);
        gui_view->draw_rect_no_text(cv::Point(roi.x,roi.y),roi.width,roi.height, 5);

    }

    if (event == CV_EVENT_LBUTTONUP && drag && !select_flag)
    {
        point2 = cv::Point(x, y);
        drag = false;
        select_flag = true;

        //calculate the chosen roi
        rs::core::rect roi;
        roi.x = std::min(point1.x,point2.x);
        roi.y = std::min(point1.y,point2.y);
        roi.width = std::abs(point1.x-point2.x);
        roi.height = std::abs(point1.y-point2.y);

        rs::object_recognition::or_configuration_interface* ptr = reinterpret_cast<rs::object_recognition::or_configuration_interface*>(param);

        //set the chosen tracking roi
        ptr->set_tracking_rois(&roi,1);
        rs::core::status st =ptr->apply_changes();

        if (st != rs::core::status_no_error)
            exit(1);

        startTracking=true;
        select_flag = false;
    }

}

void run_object_tracking(or_video_module_impl* impl, or_data_interface* or_data,
                         or_configuration_interface* or_configuration)
{
    rs::core::status st;

    // Declare data structure and size for results
    rs::object_recognition::tracking_data* tracking_data = nullptr;
    int array_size=0;

    correlated_sample_set* or_sample_set = nullptr;

    while(!is_exit && (or_sample_set = sample_set_queue.pop()) != NULL)
    {
        gui_view->set_color_image((*or_sample_set)[rs::core::stream_type::color]);

        // Run object localization or tracking processing
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

        // Retrieve tracking data from the or_data object
        st = or_data->query_tracking_result(&tracking_data, array_size);
        if (st != rs::core::status_no_error)
        {
            is_or_processing_frame = false;
            return;
        }


        // display the top bounding boxes with object name and probablity
        gui_view->draw_results(tracking_data, array_size);
        gui_view->show_results();

        is_or_processing_frame = false;


        // Display the top bounding boxes with object name and probability
        if (tracking_data)
        {
            console_view->on_object_tracking_data(tracking_data, array_size);
        }
    }
}



int main(int argc,char* argv[])
{
    rs::core::status st;
    or_utils or_utils;

    cout  << "Initializing, one moment please ..." << endl << endl;

    console_view = move(console_display::make_console_or_display());
    gui_view = move(gui_display::make_gui_or_display());

    or_video_module_impl impl;
    or_data_interface* or_data = nullptr;
    or_configuration_interface* or_configuration = nullptr;

    // Start the camera
    or_utils.init_camera(colorInfo,depthInfo,impl,&or_data,&or_configuration,true);

    // Change mode to tracking
    or_configuration->set_recognition_mode(recognition_mode::TRACKING);

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return 1;

    gui_view->initialize(colorInfo,"Tracking");
    cv::setMouseCallback(gui_view->get_win_name(),mouseHandler,or_configuration);

    cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

    // Start background thread to run recognition processing
    std::thread recognition_thread(run_object_tracking,
                                   &impl, or_data, or_configuration);
    recognition_thread.detach();

    while (!or_utils.user_request_exit())
    {
        // Get the sample set from the camera
        rs::core::correlated_sample_set* sample_set = or_utils.get_sample_set(colorInfo,depthInfo);
        gui_view->set_color_image((*sample_set)[rs::core::stream_type::color]);

        //if tracking already set and no processing now
        if(startTracking && !is_or_processing_frame)
        {
            is_or_processing_frame = true;

            // Increase image reference to hold for library processing
            (*sample_set)[rs::core::stream_type::color]->add_ref();
            (*sample_set)[rs::core::stream_type::depth]->add_ref();
            // Push the sample set to queue
            sample_set_queue.push(sample_set);

        }
        gui_view->show_results();

    }

    // Close the camera
    or_utils.stop_camera();
    cout << "-------- Stopping --------" << endl;

    return 0;
}
