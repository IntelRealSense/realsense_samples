// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include "or_data_interface.h"
#include "or_configuration_interface.h"

class Console_View
{
public:
    //getter to color cv::Mat object
    cv::Mat& get_color_cvmat();

    //draw various results
    void draw_results(rs::object_recognition::recognition_data* recognition_data,int array_size,rs::object_recognition::or_configuration_interface* or_configuration);
    void draw_results(rs::object_recognition::localization_data* localization_data,int array_size,rs::object_recognition::or_configuration_interface* or_configuration);
    void draw_results(rs::object_recognition::localization_data* localization_data,rs::core::rect* tracking_data,
                      rs::object_recognition::or_configuration_interface* or_configuration,int array_size, bool is_localize, std::vector<std::string>& objects_names);

protected:
    cv::Mat m_color;
    rs::core::image_info m_colorInfo;
};
