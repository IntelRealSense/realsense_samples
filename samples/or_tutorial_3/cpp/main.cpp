// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "camera_utils.h"
#include "console_view.h"

std::vector<std::string> objects_names;
bool is_localize=true;
bool is_tracking=false;

//after the localization has done we track the found rois
void setTracking(const rs::core::image_info& colorInfo,rs::object_recognition::or_configuration_interface* or_configuration,
                 const rs::object_recognition::localization_data* localization_data,const int array_size)
{
    //change mode to tracking
    or_configuration->set_recognition_mode(rs::object_recognition::recognition_mode::TRACKING);
    // prepare rois for the tracking
    rs::core::rectF32* trackingRois = new rs::core::rectF32[array_size];
    for (int i = 0; i< array_size; i++)
    {
        //convert form relative roi to roi in image coordinates
        rs::core::rectF32 roi= {(float)localization_data[i].roi.x/colorInfo.width,(float)localization_data[i].roi.y/colorInfo.height,
                                (float)localization_data[i].roi.width/colorInfo.width,(float)localization_data[i].roi.height/colorInfo.height
                               };
        trackingRois[i] =roi;
    }
    // set the rois
    or_configuration->set_tracking_rois(trackingRois,array_size);
    rs::core::status st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        exit(1);

    // set the state accordingly
    is_localize = false;
    is_tracking = true;
}

int main(int argc,char* argv[])
{
    rs::core::status st;
    // handler for camera staff
    camera_utils cu;
    // handler for GUI staff
    Console_View console_view;

    //declare data structure and size for results
    rs::object_recognition::localization_data* localization_data = nullptr;
    rs::core::rect* tracking_data = nullptr;
    int array_size=0;

    rs::core::image_info colorInfo,depthInfo;
    rs::object_recognition::or_video_module_impl impl;
    rs::object_recognition::or_data_interface* or_data = nullptr;
    rs::object_recognition::or_configuration_interface* or_configuration = nullptr;

    //start the camera
    cu.init_camera(colorInfo,depthInfo,impl,&or_data,&or_configuration);

    //change mode to localization
    or_configuration->set_recognition_mode(rs::object_recognition::recognition_mode::LOCALIZATION);
    //set the localization mechanism to use CNN
    or_configuration->set_localization_mechanism(rs::object_recognition::localization_mechanism::CNN);
    //ignore all objects under 0.7 probabilty (confidence)
    or_configuration->set_recognition_confidence(0.7);

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return 1;

    const char ESC_KEY = 27;
    // handling opencv callback
    // cv::setMouseCallback(console_view.get_win_name(),mouseHandler,or_configuration);

    while (true)
    {
        // get the sample set from the camera
        rs::core::correlated_sample_set* sample_set = cu.get_sample_set(colorInfo,depthInfo);
        // cu.copy_color_to_cvmat(console_view.get_color_cvmat());
        if (is_localize || is_tracking)
        {
            // after the sample is ready we can process the frame as well
            st = impl.process_sample_set_sync(sample_set);
            if (st != rs::core::status_no_error)
                return 1;

            if (is_localize)
            {
                //retrieve localization data from the or_data object
                st =or_data->query_localization_result(&localization_data, array_size);
                if (st != rs::core::status_no_error)
                    return 1;
            }
            else
            {
                //retrieve tracking data from the or_data object
                st =or_data->query_tracking_result(&tracking_data, array_size);
                if (st != rs::core::status_no_error)
                    return 1;
            }

            if (is_localize)
            {
                if(localization_data && array_size != 0)
                {
                    console_view.draw_results(localization_data, tracking_data,
                                              or_configuration, array_size, is_localize, objects_names);
                }
                //after localization has done we want to track the found objects.
                setTracking(colorInfo,or_configuration,localization_data,array_size);
            }
            else
            {
                // display the top bounding boxes with object name and probablity
                if(tracking_data)
                {
                    console_view.draw_results(localization_data,tracking_data,
                                              or_configuration, array_size,is_localize,objects_names);
                }
            }
        }
    }

    // close the camera as well
    cu.stop_camera();

    return 0;
}
