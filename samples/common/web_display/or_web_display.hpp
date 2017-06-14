// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <json/json.hpp>

#include "or_data_interface.h"
#include "or_configuration_interface.h"

#include "transporter_proxy.hpp"

using namespace std;
using namespace cv;
using namespace rs::core;
using namespace rs::object_recognition;
using nlohmann::json;

namespace web_display
{

struct pose_compare
{
    bool operator()(const rs::core::point3dF32 &lhs, const rs::core::point3dF32 &rhs) const
    {
        return lhs.x < rhs.x || lhs.z < rhs.z;
    }
};


class or_web_display
{
public:
    or_web_display(const char *path, int port, bool jpeg)
    {
        m_transporter_proxy = &(transporter_proxy::getInstance(path, port, jpeg));
    }

    void on_or_update(uchar* pose_data,
                      localization_data *localization_data, int array_size,
                      or_configuration_interface *or_configuration)
    {
        if (array_size == 0 || !localization_data) return;

        json filter_OR_data = construct_or_json(pose_data, localization_data, array_size, or_configuration,
                                     true);
        m_transporter_proxy->send_json_data(filter_OR_data);

        auto unfilter_OR_data = construct_or_json(pose_data, localization_data, array_size, or_configuration,
                                false);
        m_transporter_proxy->send_json_data(unfilter_OR_data);

    }

    void on_object_list(vector<string> obj_name_list)
    {
        json result_json;
        json &l = result_json["list"];
        for (unsigned int i = 0; i < obj_name_list.size(); i++)
        {
            l += obj_name_list[i];
        }
        result_json["type"] = "object_recognition_label_list";

        m_transporter_proxy->send_json_data(result_json);
    }

    void on_object_localization_data(localization_data *localization_data,
                                     int array_size, or_configuration_interface *or_configuration)
    {
        if (!localization_data)
        {
            return;
        }
        auto unfilter_OR_data = construct_localization_json(localization_data, array_size, or_configuration);
        m_transporter_proxy->send_json_data(unfilter_OR_data);

    }

    void on_object_recognition_data(recognition_data *recognition_data,
                                    int array_size, or_configuration_interface *or_configuration)
    {
        if (array_size == 0 || !recognition_data) return;


        auto unfilter_OR_data = construct_recognition_json(recognition_data, array_size, or_configuration);
        m_transporter_proxy->send_json_data(unfilter_OR_data);

    }

    void on_object_tracking_data(localization_data *localization_data,
                                 tracking_data *tracking_data, or_configuration_interface *or_configuration,
                                 int array_size, bool is_localize, std::vector<std::string> &objects_names)
    {
        if (array_size == 0 || (!localization_data && !tracking_data))
            return;
        auto unfilter_OR_data = construct_tracking_json(localization_data, tracking_data, or_configuration,
                                array_size, is_localize, objects_names);
        m_transporter_proxy->send_json_data(unfilter_OR_data);

    }

    void on_rgb_frame(uint64_t ts_micros, int width, int height, const void *data)
    {
        m_transporter_proxy->on_rgb_frame(ts_micros, width, height, data);

    }


private:
    transporter_proxy *m_transporter_proxy;

    json construct_or_json(uchar* pose_data,
                                  rs::object_recognition::localization_data *localization_data, int array_size,
                                  rs::object_recognition::or_configuration_interface *or_configuration,
                                  bool needFilter)
    {
        json obj_list_json;
        for (int i = 0; i < array_size; i++)
        {
            string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
            float confidence = localization_data[i].probability;

            if (confidence < 0.80)
            {
                continue;
            }

            rs::core::rect rect = localization_data[i].roi;
            auto center = localization_data[i].object_center;

            rs::core::point3dF32 actual_pose = camera_to_world_coordinates(pose_data, center.coordinates.x, center.coordinates.y,
                                               center.coordinates.z);

            // We got object's real center and its name, now lets do a filter before send to UI.
            if (needFilter && filter_objects_based_on_pose(actual_pose, objName))
            {
                //since this object already in the display list, filter it out
                continue;
            }

            float *or_pose = new float[3];
            or_pose[0] = actual_pose.x;
            or_pose[1] = actual_pose.y;
            or_pose[2] = actual_pose.z;

            // Construct json
            json value;
            //label and confidence
            value["label"] = objName;
            value["confidence"] = confidence;

            // pose
            json &p = value["pose"];
            for (int i = 0; i < 3; i++)
            {
                p += or_pose[i];
            }

            json &r = value["rectangle"];
            r += rect.x;
            r += rect.y;
            r += rect.width;
            r += rect.height;

            // Put json obj to the json list
            obj_list_json += value;
        }

        json result_json;
        result_json["Object_result"] = obj_list_json;
        if (needFilter)
        {
            result_json["type"] = "object_recognition";
        }
        else
        {
            result_json["type"] = "unfilter_object_recognition";
        }

        return result_json;
    }

    json construct_localization_json(rs::object_recognition::localization_data *localization_data,
                                            int array_size, or_configuration_interface *or_configuration)
    {
        json obj_list_json;
        for (int i = 0; i < array_size; i++)
        {
            string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
            float confidence = localization_data[i].probability;
            rs::core::rect rect = localization_data[i].roi;

            float *center_coord = new float[3];
            center_coord[0] = localization_data[i].object_center.coordinates.x;
            center_coord[1] = localization_data[i].object_center.coordinates.y;
            center_coord[2] = localization_data[i].object_center.coordinates.z;

            // Construct json
            json value;

            // Label and confidence
            value["label"] = objName;
            value["confidence"] = confidence;

            // Pose
            json &p = value["centerCoord"];
            for (int i = 0; i < 3; i++)
            {
                p += center_coord[i];
            }

            json &r = value["rectangle"];
            r += rect.x;
            r += rect.y;
            r += rect.width;
            r += rect.height;

            // Put json obj to the json list
            obj_list_json += value;

        }

        json result_json;

        if (array_size == 0)
        {
            result_json["type"] = "object_localization_none";
        }
        else
        {
            result_json["Object_result"] = obj_list_json;
            result_json["type"] = "object_localization";
        }

        return result_json;
    }

    json construct_recognition_json(recognition_data *recognition_data,
                                           int array_size, or_configuration_interface *or_configuration)
    {
        json obj_list_json;
        std::string objName = or_configuration->query_object_name_by_id(
                                  recognition_data[0].label);
        float confidence = recognition_data[0].probability;

        //Construct json
        json value;

        // Label and Confidence
        value["label"] = objName;
        value["confidence"] = confidence;

        // Put json obj to the json list
        obj_list_json += value;

        json result_json;
        result_json["Object_result"] = obj_list_json;
        result_json["type"] = "object_recognition";

        return result_json;
    }

    json construct_tracking_json(localization_data *localization_data,
                                        tracking_data *tracking_data,
                                        or_configuration_interface *or_configuration,
                                        int array_size, bool is_localize, vector<string> &objects_names)
    {
        json obj_list_json;

        // Construct json
        json result_json;

        if (is_localize)
        {
            for (int i = 0; i < array_size; i++)
            {
                string objName = or_configuration->query_object_name_by_id(localization_data[i].label);
                objects_names.emplace_back(objName);
                float confidence = localization_data[i].probability;
                rs::core::rect rect = localization_data[i].roi;

                float *center_coord = new float[3];
                center_coord[0] = localization_data[i].object_center.coordinates.x;
                center_coord[1] = localization_data[i].object_center.coordinates.y;
                center_coord[2] = localization_data[i].object_center.coordinates.z;

                // Construct json
                json value;

                // Label and confidence
                value["label"] = objName;
                value["confidence"] = confidence;

                // Pose
                json &p = value["centerCoord"];
                for (int i = 0; i < 3; i++)
                {
                    p += center_coord[i];
                }

                json &r = value["rectangle"];
                r += rect.x;
                r += rect.y;
                r += rect.width;
                r += rect.height;

                // Put json obj to the json list
                obj_list_json += value;
            }
            result_json["type"] = "object_localization";
        }
        else
        {
            for (int i = 0; i < array_size; i++)
            {
                // Construct json
                json value;
                string objName = objects_names[i];

                value["label"] = objName;
                json &r = value["rectangle"];
                r += tracking_data[i].roi.x;
                r += tracking_data[i].roi.y;
                r += tracking_data[i].roi.width;
                r += tracking_data[i].roi.height;
                // Put json obj to the json list
                obj_list_json += value;
            }
            cout << endl;
            result_json["type"] = "object_tracking";
        }


        result_json["Object_result"] = obj_list_json;
        return result_json;
    }

    // Transform coordinates from the camera current position to the "world coordinates"
    rs::core::point3dF32 camera_to_world_coordinates(
        uchar* pose_data, float x, float y, float z)
    {
        cv::Mat poseMat(4, 4, CV_32FC1, pose_data);
        cv::Mat_<float> cameraPtMat(4, 1);
        cameraPtMat(0, 0) = x / 1000;  // mm -> meters
        cameraPtMat(1, 0) = y / 1000;  // mm -> meters
        cameraPtMat(2, 0) = z / 1000;  // mm -> meters
        cameraPtMat(3, 0) = 1.0f;

        cv::Mat_<float> worldPtMat = poseMat * cameraPtMat;

        rs::core::point3dF32 world_pt;
        world_pt.x = worldPtMat(0, 0);
        world_pt.y = worldPtMat(1, 0);
        world_pt.z = worldPtMat(2, 0);

        return world_pt;
    }

    // Drop the new obj if it's already in the list
    map<rs::core::point3dF32, string, pose_compare> m_pose_objects_map;

    bool filter_objects_based_on_pose(rs::core::point3dF32 obj_center, string obj_label)
    {
        for (auto const &ent : m_pose_objects_map)
        {
            if (abs(obj_center.x - ent.first.x) < 0.5 && abs(obj_center.y - ent.first.y) < 0.5
                    && abs(obj_center.z - ent.first.z) < 0.5)
            {
                // Filter it out
                return true;
            }

        }
        m_pose_objects_map[obj_center] = obj_label;

        return false;
    }
};

std::unique_ptr<web_display::or_web_display> make_or_web_display(string &sample_name, int port, bool jpeg_compression)
{
    size_t found = sample_name.find_last_of("/");
    string path = sample_name + "_browser";
    if(found != string::npos)
    {
        path = sample_name.substr(found+1) + "_browser";
    }

    // If file content not found locally, look for it under 'share'
    struct stat info;
    if( stat( path.c_str(), &info ) != 0 )
    {
        path = INSTALL_PREFIX "/share/librealsense/samples/" + path;
    }

    cout << endl << "web folder path: " << path << endl;
    return unique_ptr<or_web_display> { new web_display::or_web_display(path.c_str(), port, jpeg_compression) };
}

} // end namespace web_display
