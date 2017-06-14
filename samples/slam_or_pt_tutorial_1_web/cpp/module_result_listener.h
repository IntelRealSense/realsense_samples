// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <list>
#include <algorithm>
#include <librealsense/rs.hpp>
#include <sys/types.h>
#include <sys/stat.h>

#include "or_data_interface.h"
#include "or_configuration_interface.h"
#include "rs_sdk.h"
#include "person_tracking_video_module_factory.h"
#include "slam/utils.h"

#include "or_console_display.hpp"
#include "slam_web_display.hpp"
#include "or_web_display.hpp"
#include "pt_web_display.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;
using namespace Intel::RealSense::PersonTracking;

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
    virtual void on_person_tracking_started(int depth_frame_number) = 0;
    virtual void on_person_tracking_finished(correlated_sample_set& pt_sample_set, rs::person_tracking::person_tracking_video_module_interface* ptModule) = 0;
    virtual void on_slam_restart() = 0;
    virtual void on_slam_pose_update(tracking_accuracy tracking, PoseMatrix4f& cameraPose, int depth_frame_number) = 0;
    virtual void on_slam_occupancy_update(float scale, int count, const int* tiles, float occupancy_res) = 0;
    virtual void on_slam_fps(char* type, float fisheye, float depth, float accelerometer, float gyroscope) = 0;
    virtual void on_slam_fisheye_update(uint64_t ts_micros, int width, int height,
                                        const void* data) = 0;
    virtual void on_rgb_frame_update(uint64_t ts_micros, int width, int height,
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
        //display localization_data
        if(recognition_data && array_size != 0)
        {
            or_console_view->on_object_recognition_data(recognition_data, array_size,or_configuration);
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
        if(array_size != 0 && m_frame_pose_map.find(depth_frame_number) != m_frame_pose_map.end())
        {
            PoseMatrix4f cameraPose = m_frame_pose_map[depth_frame_number];
            printf("camera pose for Object Recognition frame:(% 5.2f, % 5.2f, % 5.2f)\n",
                   cameraPose.m_data[3], // x
                   cameraPose.m_data[7], // y
                   cameraPose.m_data[11]); // z

            m_frame_pose_map.erase(depth_frame_number);

            or_web_view->on_or_update((uchar*)cameraPose.m_data, localization_data,
                                      array_size, or_configuration);
        }

        //display localization_data
        if(localization_data && array_size != 0)
        {
            or_console_view->on_object_localization_data(localization_data, array_size,or_configuration);
        }
        if(or_sample_set[stream_type::color])
            or_sample_set[stream_type::color]->release();
        if(or_sample_set[stream_type::depth])
            or_sample_set[stream_type::depth]->release();

        or_sample_set[stream_type::color] = nullptr;
        or_sample_set[stream_type::depth] = nullptr;
    }

    void on_person_tracking_started(int depth_frame_number)
    {
        m_waiting_pose_frame_list.push_back(depth_frame_number);
    }

    void on_person_tracking_finished(correlated_sample_set& pt_sample_set, rs::person_tracking::person_tracking_video_module_interface* ptModule)
    {
        int depth_frame_number = pt_sample_set[stream_type::depth]->query_frame_number();
        if(m_frame_pose_map.find(depth_frame_number) != m_frame_pose_map.end())
        {
            PoseMatrix4f cameraPose = m_frame_pose_map[depth_frame_number];
            pt_web_view->on_pt_update((uchar*)cameraPose.m_data, ptModule);
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
        ui_request_restart = false;
        slam_web_view->on_reset_competed();
        cout << "SLAM Restarted------------------------------" << endl;
    }

    void on_slam_pose_update(tracking_accuracy tracking, PoseMatrix4f& cameraPose, int depth_frame_number) override
    {
        std::list<int>::iterator findIter = std::find(m_waiting_pose_frame_list.begin(),
                                            m_waiting_pose_frame_list.end(), depth_frame_number);

        if(findIter != m_waiting_pose_frame_list.end())
        {
            m_frame_pose_map[depth_frame_number] = cameraPose;
            m_waiting_pose_frame_list.remove(depth_frame_number);
        }
        slam_web_view->on_pose((int)tracking, cameraPose.m_data);

    }

    void on_slam_occupancy_update(float scale, int count, const int* tiles, float occupancy_res) override
    {
        float x_coord = tiles[0] * occupancy_res; // x coordinate in meters
        float z_coord = tiles[1] * occupancy_res; // z coordinate in meters
        int occ_pct = tiles[2]; // %occupancy
        printf("occ. map: tiles updated=%d\ttile_0: pos=(% 5.2f, % 5.2f) occupancy=%2d%%\n",
               count, x_coord, z_coord, occ_pct);

        slam_web_view->on_occupancy(scale, count, tiles);
    }

    void on_slam_fisheye_update(uint64_t ts_micros, int width, int height,
                                const void* data) override
    {
        slam_web_view->on_fisheye_frame(ts_micros, width, height, data);
    }

    void on_rgb_frame_update(uint64_t ts_micros, int width, int height,
                             const void* data) override
    {
        slam_web_view->on_rgb_frame(ts_micros, width, height, data);
    }

    void on_slam_fps(char* type, float fisheye, float depth, float accelerometer, float gyroscope) override
    {
        slam_web_view->on_fps(type, fisheye, depth, accelerometer, gyroscope);
    }

    void start(string sample_name)
    {
        //Launch GUI
        or_web_view = move(web_display::make_or_web_display(sample_name, 8000, true));
        slam_web_view = move(web_display::make_slam_web_display(sample_name, 8000, true));
        pt_web_view = move(web_display::make_pt_web_display(sample_name, 8000, true));
        or_console_view = move(console_display::make_console_or_display());

        reset_callback = [&]()
        {
            std::cout << "rs_sp: calling reset" << endl;
            ui_request_restart = true;
        };
        stop_callback = [&]()
        {
            ui_request_stop = true;
        };
        controls.reset = reset_callback;
        controls.stop = stop_callback;

        slam_web_view->set_control_callbacks(controls);
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
    unique_ptr<web_display::or_web_display>  or_web_view;
    unique_ptr<web_display::pt_web_display>  pt_web_view;
    unique_ptr<web_display::slam_web_display>  slam_web_view;

    bool ui_request_stop;
    bool ui_request_restart;
    display_controls controls;
    std::function<void()> reset_callback;
    std::function<void()> stop_callback;
    unique_ptr<console_display::or_console_display> or_console_view;

    map<int, PoseMatrix4f> m_frame_pose_map;
    list<int> m_waiting_pose_frame_list;
};
