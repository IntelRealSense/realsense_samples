// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <iomanip>

#include "or_data_interface.h"
#include "or_configuration_interface.h"

using namespace std;
using namespace cv;
using namespace rs::core;
using namespace rs::object_recognition;

namespace console_display
{

class or_console_display
{
public:
    or_console_display()
    {
    }

    or_console_display(int color_width, int color_height)
    {
        cv::namedWindow(or_console_display::IMAGE_WINDOW_NAME, CV_WINDOW_NORMAL);
        cv::resizeWindow(or_console_display::IMAGE_WINDOW_NAME, color_width, color_height);
    }

    /**
     * @brief Parse and display recognition result on screen
     *
     * @param[in] recognition_data The output of the OR recognition process.
     * @param[in] array_size The size of the recognition_data array (number of recognized classes).
     * @param[in] or_configuration The configuration of the OR Recognition process.
     */
    void on_object_recognition_data(recognition_data *recognition_data, int array_size,
                                    or_configuration_interface *or_configuration)
    {
        std::string objName = or_configuration->query_object_name_by_id(
                                  recognition_data[0].label);
        float confidence = recognition_data[0].probability;
        std::stringstream textToWrite;
        textToWrite.precision(2);
        textToWrite << confidence;

        // Print out OR result
        cout << left << setw(18) << "Label" << "Confidence" << endl;
        cout << left << setw(18) << "-----" << "----------" << endl;
        cout << left << setw(18) << objName << textToWrite.str() << endl;
        cout << endl;
    }

    /**
     * @brief Parse and display localization result on screen
     *
     * @param[in] localization_data The output of the OR Localization process.
     * @param[in] array_size The size of the localization_data array (number of localized objects).
     * @param[in] or_configuration The configuration of the OR Localization process.
     */
    void on_object_localization_data(localization_data *localization_data, int array_size,
                                     or_configuration_interface *or_configuration)
    {
        cout << left << setw(18) << "Label" << setw(18) << "Confidence"
             << setw(28) << "Object Center" << "Coordinate" << endl;
        cout << left << setw(18) << "-----" << setw(18) << "----------"
             << setw(28) << "---------------" << "------------" << endl;
        for (int i = 0; i < array_size; i++)
        {
            string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
            float confidence = localization_data[i].probability;
            rs::core::rect rect = localization_data[i].roi;
            std::stringstream confidenceText;
            confidenceText.precision(2);
            confidenceText << confidence;

            std::stringstream centerText;
            centerText.precision(2);
            centerText << "(" << localization_data[i].object_center.coordinates.x / 1000.0 << "," <<
                       localization_data[i].object_center.coordinates.y / 1000.0 << "," <<
                       localization_data[i].object_center.coordinates.z / 1000.0 << ")";

            cout << left << setw(18) << objName << setw(18) << confidenceText.str() <<
                 setw(28) << centerText.str() <<
                 "(" << rect.x << "," << rect.y << ")"
                 << " (" << rect.x + rect.width << "," << rect.y + rect.height << ")" << endl;

        }
        cout << endl;
    }

    /**
     * @brief Parse and display localization or tracking result on screen
     *
     * @remark Assumes objectsNames is empty
     *
     * @param[in] localization_data The output of the OR Localization process.
     * @param[in] tracking_data The output of the OR Tracking process.
     * @param[in] or_configuration The configuration of the OR process.
     * @param[in] array_size The size of the localization_data array (number of localized objects).
     * @param[in] is_localize A boolean showing if the treated data should be from localization or tracking
     * @param[in] objects_names The names of all the tracked objects
     */
    void on_object_tracking_data(localization_data *localization_data, tracking_data *tracking_data,
                                 or_configuration_interface *or_configuration, int array_size,
                                 bool is_localize, std::vector<std::string> &objects_names)
    {
        if (is_localize)
        {
            std::cout << std::endl << "localization object information:" << std::endl;

            // Localization result
            cout << left << setw(18) << "Label" << setw(18) << "Confidence"
                 << setw(28) << "Object Center" << "Coordinate" << endl;
            cout << left << setw(18) << "-----" << setw(18) << "----------"
                 << setw(28) << "---------------" << "------------" << endl;
            for (int i = 0; i < array_size; i++)
            {
                string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
                objects_names.emplace_back(objName);
                float confidence = localization_data[i].probability;
                rs::core::rect rect = localization_data[i].roi;
                std::stringstream confidenceText;
                confidenceText.precision(2);
                confidenceText << confidence;

                std::stringstream centerText;
                centerText.precision(2);
                centerText << "(" << localization_data[i].object_center.coordinates.x / 1000.0 << "," <<
                           localization_data[i].object_center.coordinates.y / 1000.0 << "," <<
                           localization_data[i].object_center.coordinates.z / 1000.0 << ")";

                cout << left << setw(18) << objName << setw(18) << confidenceText.str() <<
                     setw(28) << centerText.str() <<
                     "(" << rect.x << "," << rect.y << ")"
                     << " (" << rect.x + rect.width << "," << rect.y + rect.height << ")" << endl;

            }
            cout << endl << "Will track all of these objects." << endl << endl;
        }
        else
        {
            // Tracking result
            cout << left << setw(18) << "Label" << "Coordinate" << endl;
            cout << left << setw(18) << "-----" << "------------" << endl;
            for (int i = 0; i < array_size; i++)
            {
                string objName = objects_names[i];
                cout << left << setw(18) << objName
                     << "(" << tracking_data[i].roi.x << "," << tracking_data[i].roi.y << ")"
                     << " (" << tracking_data[i].roi.x + tracking_data[i].roi.width << ","
                     << tracking_data[i].roi.y + tracking_data[i].roi.height << ")" << endl;
            }
            cout << endl;
        }
    }


    /**
     * @brief Parse and display tracking result on screen
     *
     * @param[in] tracking_data The output of the OR Tracking process.
     * @param[in] array_size The size of the localization_data array (number of localized objects).
     */
    void on_object_tracking_data(tracking_data *tracking_data, int array_size)
    {

            // Tracking result
            cout << left << setw(18) << "Label" << "Coordinate" << endl;
            cout << left << setw(18) << "-----" << "------------" << endl;
            for (int i = 0; i < array_size; i++)
            {
                cout << left << setw(18) << i
                     << "(" << tracking_data[i].roi.x << "," << tracking_data[i].roi.y << ")"
                     << " (" << tracking_data[i].roi.x + tracking_data[i].roi.width << ","
                     << tracking_data[i].roi.y + tracking_data[i].roi.height << ")" << endl;
            }
            cout << endl;
    }





    /**
     * @brief Render color image on OpenCV window
     *
     * @param colorImage the color image will be drawed
     */
    void render_color_frames(rs::core::image_interface *colorImage)
    {
        cv::Mat renderImage = Image2Mat(colorImage);
        cv::imshow(or_console_display::IMAGE_WINDOW_NAME, renderImage);
        cv::waitKey(1);
    }

    /**
     * @brief Convert Image to cv::Mat
     *
     * @param image the color image need to be converted
     * @return the converted cv::Mat
     */
    cv::Mat Image2Mat(rs::core::image_interface *image)
    {
        cv::Mat mat;
        switch (image->query_info().format)
        {
        case rs::core::pixel_format::rgba8:
            mat = cv::Mat(image->query_info().height, image->query_info().width, CV_8UC4,
                          (void *) (image->query_data())).clone();
            cv::cvtColor(mat, mat, CV_RGBA2BGR);
            break;
        case rs::core::pixel_format::bgra8:
            mat = cv::Mat(image->query_info().height, image->query_info().width, CV_8UC4,
                          (void *) (image->query_data())).clone();
            cv::cvtColor(mat, mat, CV_BGRA2BGR);
            break;
        case rs::core::pixel_format::bgr8:
            mat = cv::Mat(image->query_info().height, image->query_info().width, CV_8UC3,
                          (void *) (image->query_data())).clone();
            break;
        case rs::core::pixel_format::rgb8:
            mat = cv::Mat(image->query_info().height, image->query_info().width, CV_8UC3,
                          (void *) (image->query_data())).clone();
            cv::cvtColor(mat, mat, CV_RGB2BGR);
            break;
        default:
            std::runtime_error("unsupported color format");
        }
        return mat;
    }

protected:
    cv::Mat m_color;
    rs::core::image_info m_colorInfo;
    static const string IMAGE_WINDOW_NAME;
};

const string or_console_display::IMAGE_WINDOW_NAME = "Color image";

std::unique_ptr<console_display::or_console_display> make_console_or_display()
{
    return unique_ptr<or_console_display> { new console_display::or_console_display() };
}
}
