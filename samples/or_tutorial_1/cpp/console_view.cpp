// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <iomanip>
#include "console_view.h"

using namespace std;

cv::Mat& Console_View::get_color_cvmat()
{
    return m_color;
}

void Console_View::draw_results(rs::object_recognition::localization_data* localization_data,
                                int array_size,rs::object_recognition::or_configuration_interface* or_configuration)
{
    //print out localization result on screen
    cout  << left << setw(18) << "Label"  << setw(18) << "Confidence"
          << setw(28) << "Object Center" << "Coordinate" << endl;
    cout << left << setw(18) << "-----" << setw(18) << "----------"
         << setw(28) << "---------------" << "------------" << endl;
    for (int i = 0; i< array_size; i++)
    {
        string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
        float confidence = localization_data[i].probability;
        rs::core::rect rect = localization_data[i].roi;
        std::stringstream confidenceText;
        confidenceText.precision(2);
        confidenceText << confidence;

        std::stringstream centerText;
        centerText.precision(2);
        centerText << "(" << localization_data[i].object_center.x <<"," <<
                   localization_data[i].object_center.y << "," <<
                   localization_data[i].object_center.z << ")";

        cout << left << setw(18) << objName  << setw(18)<< confidenceText.str() <<
             setw(28) << centerText.str() <<
             "(" << rect.x <<"," << rect.y << ")"
             << " (" << rect.x + rect.width <<"," << rect.y+rect.height << ")"<< endl;

    }
    cout << endl;
}

void Console_View::draw_results(rs::object_recognition::recognition_data* recognition_data,
                                int array_size,rs::object_recognition::or_configuration_interface* or_configuration)
{
    //print out recognition result on screen
    std::string objName = or_configuration->query_object_name_by_id(
                              recognition_data[0].label);
    float confidence = recognition_data[0].probability;
    std::stringstream textToWrite;
    textToWrite.precision(2);
    textToWrite << confidence;

    //print out OR result
    cout  << left << setw(18) << "Label"  << "Confidence" << endl;
    cout << left << setw(18) << "-----" << "----------" << endl;
    cout << left << setw(18) << objName << textToWrite.str() << endl;
    cout << endl;
}

void Console_View::draw_results(rs::object_recognition::localization_data* localization_data,
                                rs::core::rect* tracking_data,
                                rs::object_recognition::or_configuration_interface* or_configuration,
                                int array_size, bool is_localize, std::vector<std::string>& objects_names)
{
    if (is_localize)
    {
        std::cout << std::endl << "localization object information:"  << std::endl;

        //print out localization result on screen
        cout  << left << setw(18) << "Label"  << setw(18) << "Confidence"
              << setw(28) << "Object Center" << "Coordinate" << endl;
        cout << left << setw(18) << "-----" << setw(18) << "----------"
             << setw(28) << "---------------" << "------------" << endl;
        for (int i = 0; i< array_size; i++)
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
            centerText << "(" << localization_data[i].object_center.x <<"," <<
                       localization_data[i].object_center.y << "," <<
                       localization_data[i].object_center.z << ")";

            cout << left << setw(18) << objName  << setw(18)<< confidenceText.str() <<
                 setw(28) << centerText.str() <<
                 "(" << rect.x <<"," << rect.y << ")"
                 << " (" << rect.x + rect.width <<"," << rect.y+rect.height << ")"<< endl;

        }
        cout << endl << "Will track all of these objects."  << endl << endl;
    }
    else
    {
        //print out tracking result on screen
        cout  << left << setw(18) << "Label" << "Coordinate" << endl;
        cout << left << setw(18) << "-----" << "------------" << endl;
        for (int i = 0; i< array_size; i++)
        {
            string objName = objects_names[i];
            cout << left << setw(18) << objName
                 << "(" << tracking_data[i].x <<"," << tracking_data[i].y << ")"
                 << " (" << tracking_data[i].x + tracking_data[i].width <<","
                 << tracking_data[i].y+tracking_data[i].height << ")"<< endl;
        }
        cout << endl;
    }
}
