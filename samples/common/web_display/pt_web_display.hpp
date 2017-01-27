// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <set>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>

#include <jsoncpp/json/json.h>
#include <opencv2/opencv.hpp>

#include "person_tracking_video_module_factory.h"

#include "transporter_proxy.hpp"

using namespace std;
using namespace rs::person_tracking;
using namespace Intel::RealSense::PersonTracking;

namespace web_display
{
struct PoseCompare
{
    bool operator()(const rs::core::point3dF32 &lhs, const rs::core::point3dF32 &rhs) const
    {
        return lhs.x < rhs.x || lhs.z < rhs.z;
    }
};

class pt_web_display
{
public:
    pt_web_display(const char *path, int port, bool jpeg)
    {
        m_transporter_proxy = &(transporter_proxy::getInstance(path, port, jpeg));
    }

    void on_pt_update(uchar* pose_data, rs::person_tracking::person_tracking_video_module_interface* ptModule)
    {
        Json::Value value(Json::objectValue);
        if(ConstructPTJson(pose_data, ptModule, value))
        {
            m_transporter_proxy->send_json_data(value);
        }
    }

    void on_PT_tracking_update(rs::person_tracking::person_tracking_video_module_interface* ptModule, int cumulative_total)
    {
        auto json_to_send = ConstructPtTrackingJson(ptModule, cumulative_total);
        m_transporter_proxy->send_json_data(json_to_send);
    }

    void on_PT_tracking_update(rs::person_tracking::person_tracking_video_module_interface* ptModule)
    {
        auto json_to_send = ConstructPtTrackingJson(ptModule, 0);
        m_transporter_proxy->send_json_data(json_to_send);
    }

    void on_pt_head_pose_update(rs::person_tracking::person_tracking_video_module_interface* ptModule)
    {
        auto json_to_send = ConstructPtHeadPosePersonOrientationJson(ptModule);
        m_transporter_proxy->send_json_data(json_to_send);
    }

    void on_pt_pointing_gesture_update(rs::person_tracking::person_tracking_video_module_interface* ptModule)
    {
        auto json_to_send = ConstructPtPointingGestureJson(ptModule);
        m_transporter_proxy->send_json_data(json_to_send);

    }
    void on_rgb_frame(uint64_t ts_micros, int width, int height, const void* data)
    {
        m_transporter_proxy->on_rgb_frame(ts_micros, width, height, data);
    }

    void set_control_callbacks(display_controls controls)
    {
        m_transporter_proxy->set_control_callbacks(controls);
    }

private:
    transporter_proxy *m_transporter_proxy;

    Json::Value ConstructPtTrackingJson(
        person_tracking_video_module_interface *ptModule, int cumulative_total)
    {
        Json::Value obj_list_json(Json::arrayValue);
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        for (int index = 0; index < trackingData->QueryNumberOfPeople(); index++)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, index);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking *personTrackingData = personData->QueryTracking();
                Intel::RealSense::PersonTracking::PersonTrackingData::BoundingBox2D box = personTrackingData->Query2DBoundingBox();
                Intel::RealSense::PersonTracking::PersonTrackingData::PointCombined centerMass = personTrackingData->QueryCenterMass();

                int id = personTrackingData->QueryId();

                // Construct json
                Json::Value value(Json::objectValue);
                // person id
                value["pid"] = id;

                value["cumulative_total"] = cumulative_total;

                value["person_bounding_box"]["x"] = box.rect.x;
                value["person_bounding_box"]["y"] = box.rect.y;
                value["person_bounding_box"]["w"] = box.rect.w;
                value["person_bounding_box"]["h"] = box.rect.h;

                value["center_mass_image"]["x"] = centerMass.image.point.x;
                value["center_mass_image"]["y"] = centerMass.image.point.y;

                value["center_mass_world"]["x"] = centerMass.world.point.x;
                value["center_mass_world"]["y"] = centerMass.world.point.y;
                value["center_mass_world"]["z"] = centerMass.world.point.z;

                if (ptModule->QueryConfiguration()->QueryRecognition()->IsEnabled()){
                     // Get recognition ID from person data
                     PersonTrackingData::PersonRecognition* recognition = personData->QueryRecognition();
                     int32_t outputRecognitionId;
                     int32_t outputTrackingId;
                     int32_t outDescriptorId;
                     auto status = recognition->RegisterUser(&outputRecognitionId, &outputTrackingId, &outDescriptorId);

                     if (status == PersonTrackingData::PersonRecognition::RegistrationSuccessful)
                     {
                         std::cout << "Registered person: " << outputRecognitionId << std::endl;
                         int rid = outputRecognitionId;
                         // Send rid to browser
                         value["rid"] = rid;
                     }
                     else if (status == PersonTrackingData::PersonRecognition::RegistrationFailedAlreadyRegistered)
                     {
                         int rid = outputRecognitionId;
                         // Send rid to browser
                         value["rid"] = rid;
                     }
                 }
                // Put json obj to the json list
                obj_list_json.append(value);
            }
        }

        if (trackingData->QueryNumberOfPeople())
        {
            Json::Value result_json(Json::objectValue);
            result_json["Object_result"] = obj_list_json;
            result_json["type"] = "person_tracking";

            return result_json;
        }

        return obj_list_json;
    }

    Json::Value ConstructPtHeadPosePersonOrientationJson(
        person_tracking_video_module_interface *ptModule)
    {
        Json::Value obj_list_json(Json::arrayValue);
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->QueryNumberOfPeople() > 0)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PoseEulerAngles headAngles;
                auto headBoundingBox = personData->QueryTracking()->QueryHeadBoundingBox();
                // Construct json
                Json::Value value(Json::objectValue);

                // person id
                value["pid"] = personData->QueryTracking()->QueryId();
                if (headBoundingBox.confidence)
                {
                    value["head_bounding_box"]["x"] = headBoundingBox.rect.x;
                    value["head_bounding_box"]["y"] = headBoundingBox.rect.y;
                    value["head_bounding_box"]["w"] = headBoundingBox.rect.w;
                    value["head_bounding_box"]["h"] = headBoundingBox.rect.h;
                }

                // Print head pose (pitch, roll, yaw)
                if (personData->QueryFace()->QueryHeadPose(headAngles))
                {
                    value["head_pose"]["pitch"] = headAngles.pitch;
                    value["head_pose"]["roll"] = headAngles.roll;
                    value["head_pose"]["yaw"] = headAngles.yaw;
                }


                // Put json obj to the json list
                obj_list_json.append(value);
            }

            Json::Value result_json(Json::objectValue);
            result_json["Object_result"] = obj_list_json;
            result_json["type"] = "person_tracking";

            return result_json;
        }

        return obj_list_json;

    }

    Json::Value ConstructPtPointingGestureJson(
        person_tracking_video_module_interface *ptModule)
    {
        Json::Value obj_list_json(Json::arrayValue);
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->QueryNumberOfPeople() > 0)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PersonGestures *personGestures = personData->QueryGestures();
                Intel::RealSense::PersonTracking::PersonTrackingData::BoundingBox2D box = personData->QueryTracking()->Query2DBoundingBox();

                // Construct json
                Json::Value value(Json::objectValue);

                value["pid"] = personData->QueryTracking()->QueryId();

                value["person_bounding_box"]["x"] = box.rect.x;
                value["person_bounding_box"]["y"] = box.rect.y;
                value["person_bounding_box"]["w"] = box.rect.w;
                value["person_bounding_box"]["h"] = box.rect.h;

                if (personGestures)
                {
                    if (personGestures->IsPointing() && personGestures->QueryPointingInfo().confidence > 0)
                    {
                        value["gesture_color_coordinates"]["origin"]["x"] = personGestures->QueryPointingInfo().colorPointingData.origin.x;
                        value["gesture_color_coordinates"]["origin"]["y"] = personGestures->QueryPointingInfo().colorPointingData.origin.y;

                        value["gesture_color_coordinates"]["direction"]["x"] = personGestures->QueryPointingInfo().colorPointingData.direction.x;
                        value["gesture_color_coordinates"]["direction"]["y"] = personGestures->QueryPointingInfo().colorPointingData.direction.y;

                        value["gesture_world_coordinates"]["origin"]["x"] = personGestures->QueryPointingInfo().worldPointingData.origin.x;
                        value["gesture_world_coordinates"]["origin"]["y"] = personGestures->QueryPointingInfo().worldPointingData.origin.y;
                        value["gesture_world_coordinates"]["origin"]["z"] = personGestures->QueryPointingInfo().worldPointingData.origin.z;

                        value["gesture_world_coordinates"]["direction"]["x"] = personGestures->QueryPointingInfo().worldPointingData.direction.x;
                        value["gesture_world_coordinates"]["direction"]["y"] = personGestures->QueryPointingInfo().worldPointingData.direction.y;
                        value["gesture_world_coordinates"]["direction"]["z"] = personGestures->QueryPointingInfo().worldPointingData.direction.z;
                    }
                }

                // Put json obj to the json list
                obj_list_json.append(value);
            }

            Json::Value result_json(Json::objectValue);
            result_json["Object_result"] = obj_list_json;
            result_json["type"] = "person_tracking";

            return result_json;
        }

        return obj_list_json;
    }

    bool ConstructPTJson(uchar* pose_data,
                         person_tracking_video_module_interface *ptModule,
                         Json::Value &result_json)
    {
        Json::Value obj_list_json(Json::arrayValue);

        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->QueryNumberOfPeople() <= 0)
            return false;

        for (int index = 0; index < trackingData->QueryNumberOfPeople(); index++)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, index);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking *personTrackingData = personData->QueryTracking();
                Intel::RealSense::PersonTracking::PersonTrackingData::BoundingBox2D box = personTrackingData->Query2DBoundingBox();
                Intel::RealSense::PersonTracking::PersonTrackingData::PointCombined centerMass = personTrackingData->QueryCenterMass();

                // Convert to world coordinate
                rs::core::point3dF32 actual_pose = camera_to_world_coordinates(pose_data,
                                                   centerMass.world.point.x,
                                                   centerMass.world.point.y,
                                                   centerMass.world.point.z);

                // Construct json
                Json::Value value(Json::objectValue);

                // Filter out exist person data
                if (!filter_person_based_on_pose(actual_pose))
                {
                    float *pt_pose = new float[3];
                    pt_pose[0] = actual_pose.x;
                    pt_pose[1] = actual_pose.y;
                    pt_pose[2] = actual_pose.z;

                    //pose
                    Json::Value &p = value["pose"];
                    for (int i = 0; i < 3; i++)
                    {
                        p.append(pt_pose[i]);
                    }
                }

                int id = personTrackingData->QueryId();
                // person id
                value["pid"] = id;

                value["person_bounding_box"]["x"] = box.rect.x;
                value["person_bounding_box"]["y"] = box.rect.y;
                value["person_bounding_box"]["w"] = box.rect.w;
                value["person_bounding_box"]["h"] = box.rect.h;

                value["center_mass_image"]["x"] = centerMass.image.point.x;
                value["center_mass_image"]["y"] = centerMass.image.point.y;

                value["center_mass_world"]["x"] = centerMass.world.point.x;
                value["center_mass_world"]["y"] = centerMass.world.point.y;
                value["center_mass_world"]["z"] = centerMass.world.point.z;

                //put json obj to the json list
                obj_list_json.append(value);
            }

        }

        result_json["Object_result"] = obj_list_json;
        result_json["type"] = "person_tracking_data";

        return true;
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



    vector<rs::core::point3dF32> m_person_pose_set;

    bool filter_person_based_on_pose(rs::core::point3dF32 new_person_pose)
    {
        for (rs::core::point3dF32 &pose: m_person_pose_set)
        {
            if (abs(new_person_pose.x - pose.x) < 0.5 && abs(new_person_pose.y - pose.y) < 0.5
                    && abs(new_person_pose.z - pose.z) < 0.5)
            {
                // Filter it out
                return true;
            }

        }
        m_person_pose_set.push_back(new_person_pose);

        return false;

    }

    std::string orientation_to_string(
        Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking::PersonOrientation orientation)
    {
        typedef Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking PersonTracking;
        switch (orientation)
        {
        case PersonTracking::ORIENTATION_FRONTAL:
            return "frontal";
        case PersonTracking::ORIENTATION_45_DEGREE_RIGHT:
            return "45 degree right";
        case PersonTracking::ORIENTATION_45_DEGREE_LEFT:
            return "45 degree left";
        case PersonTracking::ORIENTATION_PROFILE_RIGHT:
            return "right";
        case PersonTracking::ORIENTATION_PROFILE_LEFT:
            return "left";
        case PersonTracking::ORIENTATION_REAR:
            return "rear";
        default:
            throw std::runtime_error("unsupported orientation value");
        }
    }
};

std::unique_ptr<web_display::pt_web_display> make_pt_web_display(string &sample_name, int port, bool jpeg_compression)
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
    return unique_ptr<pt_web_display> { new web_display::pt_web_display(path.c_str(), port, jpeg_compression) };
}

}
