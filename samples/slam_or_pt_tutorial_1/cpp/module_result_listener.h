// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <list>
#include <algorithm>
#include <librealsense/rs.hpp>

#include "or_data_interface.h"
#include "or_configuration_interface.h"
#include "slam/utils.h"
#include "or_console_display.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

class module_result_listener_interface
{
public:
    module_result_listener_interface() {}
    virtual ~module_result_listener_interface() {}
    virtual void on_object_recgnition_started(int depth_frame_number) = 0;
    virtual void on_object_recgnition_finished(correlated_sample_set& or_sample_set,
            rs::object_recognition::recognition_data* recognition_data,int array_size,
            rs::object_recognition::or_configuration_interface* or_configuration) = 0;
    virtual void on_object_localization_finished(correlated_sample_set& or_sample_set,
            rs::object_recognition::localization_data* localization_data,int array_size,
            rs::object_recognition::or_configuration_interface* or_configuration) = 0;
    virtual void on_person_tracking_finished(correlated_sample_set& pt_sample_set,
            int numPeopleInFrame, int totalPersonIncrements, bool people_changed) = 0;
    virtual void on_slam_restart() = 0;
    virtual void on_slam_pose_update(tracking_accuracy tracking, PoseMatrix4f& cameraPose, int depth_frame_number) = 0;
    virtual void on_slam_occupancy_update(float scale, int count, const int* tiles, float occupancy_res) = 0;
    virtual void on_slam_fps(char* type, float fisheye, float depth, float accelerometer, float gyroscope) = 0;
    virtual void on_slam_fisheye_update(uint64_t ts_micros, int width, int height,
                                        const void* data) = 0;
};

class module_consumer : public module_result_listener_interface
{
public:
    module_consumer() : ui_request_stop(false), ui_request_restart(false)
    {
    }

    ~module_consumer() {}

    void on_object_recgnition_started(int depth_frame_number) override
    {
        m_waiting_pose_frame_list.push_back(depth_frame_number);
    }

    void on_object_recgnition_finished(correlated_sample_set& or_sample_set,
                                       recognition_data* recognition_data,int array_size,
                                       or_configuration_interface* or_configuration) override
    {
        // Display localization_data
        if(recognition_data && array_size != 0)
        {
            or_console_display.on_object_recognition_data(recognition_data, array_size,or_configuration);
        }

        if(or_sample_set[stream_type::color])
            or_sample_set[stream_type::color]->release();
        if(or_sample_set[stream_type::depth])
            or_sample_set[stream_type::depth]->release();

        or_sample_set[stream_type::color] = nullptr;
        or_sample_set[stream_type::depth] = nullptr;
    }

    void on_object_localization_finished(correlated_sample_set& or_sample_set,
                                         localization_data* localization_data,int array_size,
                                         or_configuration_interface* or_configuration) override
    {
        int depth_frame_number = or_sample_set[stream_type::depth]->query_frame_number();
        if(m_frame_pose_map.find(depth_frame_number) != m_frame_pose_map.end())
        {
            PoseMatrix4f cameraPose = m_frame_pose_map[depth_frame_number];
            printf("camera pose for Object Recognition frame:(% 5.2f, % 5.2f, % 5.2f)\n",
                   cameraPose.m_data[3], // x
                   cameraPose.m_data[7], // y
                   cameraPose.m_data[11]); // z

            m_frame_pose_map.erase(depth_frame_number);
        }

        // Display localization_data
        if(localization_data && array_size != 0)
        {
            or_console_display.on_object_localization_data(localization_data, array_size,or_configuration);
        }

        if(or_sample_set[stream_type::color])
            or_sample_set[stream_type::color]->release();
        if(or_sample_set[stream_type::depth])
            or_sample_set[stream_type::depth]->release();

        or_sample_set[stream_type::color] = nullptr;
        or_sample_set[stream_type::depth] = nullptr;
    }

    void on_person_tracking_finished(correlated_sample_set& pt_sample_set,
                                     int numPeopleInFrame, int totalPersonIncrements, bool people_changed) override
    {
        // Display person tracking result
        if(people_changed)
        {
            cout  << left << setw(25) << "Current Frame Total"  << "Cumulative" << endl;
            cout << left << setw(25) << "--------------------" << "----------" << endl;
            cout << left << setw(25) << numPeopleInFrame << totalPersonIncrements << endl << endl;
        }
        if(pt_sample_set[stream_type::color])
            pt_sample_set[stream_type::color]->release();
        if(pt_sample_set[stream_type::depth])
            pt_sample_set[stream_type::depth]->release();

        pt_sample_set[stream_type::color] = nullptr;
        pt_sample_set[stream_type::depth] = nullptr;
    }

    void on_slam_restart() override
    {
        cout << "Restart--------------------------------" << endl;
    }

    void on_slam_pose_update(tracking_accuracy tracking, PoseMatrix4f& cameraPose, int depth_frame_number) override
    {
        printf("tracking: accuracy=%s,\ttrans=(% 5.2f, % 5.2f, % 5.2f)\n",
               tracking_tostring(tracking),
               cameraPose.m_data[3], // x
               cameraPose.m_data[7], // y
               cameraPose.m_data[11]); // z

        std::list<int>::iterator findIter = std::find(m_waiting_pose_frame_list.begin(),
                                            m_waiting_pose_frame_list.end(), depth_frame_number);

        if(findIter != m_waiting_pose_frame_list.end())
        {
            m_frame_pose_map[depth_frame_number] = cameraPose;
            m_waiting_pose_frame_list.remove(depth_frame_number);
        }

    }

    void on_slam_occupancy_update(float scale, int count, const int* tiles, float occupancy_res) override
    {
        float x_coord = tiles[0] * occupancy_res; // x coordinate in meters
        float z_coord = tiles[1] * occupancy_res; // z coordinate in meters
        int occ_pct = tiles[2]; // %occupancy
        printf("occ. map: tiles updated=%d\ttile_0: pos=(% 5.2f, % 5.2f) occupancy=%2d%%\n",
               count, x_coord, z_coord, occ_pct);
    }

    void on_slam_fisheye_update(uint64_t ts_micros, int width, int height,
                                const void* data) override
    {
    }

    void on_slam_fps(char* type, float fisheye, float depth, float accelerometer, float gyroscope) override
    {
    }

    void start()
    {

    }

    void stop()
    {
    }

    bool get_ui_request_restart()
    {
        return ui_request_restart;
    }

    bool get_ui_request_stop()
    {
        return ui_request_stop;
    }

    char* tracking_tostring(const tracking_accuracy accuracy)
    {
        switch (accuracy)
        {
        case tracking_accuracy::failed:
            return (char*)"fail";
        case tracking_accuracy::low:
            return (char*)"low ";
        case tracking_accuracy::medium:
            return (char*)"med.";
        case tracking_accuracy::high:
            return (char*)"high";
        default:
            return (char*)"????";
        }
    }

private:
    bool ui_request_stop;
    bool ui_request_restart;
    console_display::or_console_display or_console_display;

    map<int, PoseMatrix4f> m_frame_pose_map;
    list<int> m_waiting_pose_frame_list;
};
