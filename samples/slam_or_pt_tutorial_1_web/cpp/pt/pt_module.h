// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <sys/stat.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>

#include <algorithm>
#include <set>
#include <iomanip>
#include <librealsense/rs.hpp>
#include <signal.h>
#include "rs_sdk.h"
#include "person_tracking_video_module_factory.h"

namespace RS = Intel::RealSense;
using namespace RS::PersonTracking;
using namespace std;
using namespace rs::core;

class PT_module
{
public:
    PT_module(module_result_listener_interface* module_listener) :  lastPersonCount(0),
        totalPersonIncrements(0), prevPeopleInFrame(-1), prevPeopleTotal(-1),
        id_set(nullptr), pt_initialized(false), pt_is_running(false), m_stopped(false),
        m_module_listener(module_listener)
    {
        dataDir = get_data_files_path();
        // Create person tracker video module
        ptModule.reset(rs::person_tracking::person_tracking_video_module_factory::
                       create_person_tracking_video_module(dataDir.c_str()));

        m_sample_set = new rs::core::correlated_sample_set();
    }

    int init_pt(rs::core::video_module_interface::supported_module_config& cfg,
                rs::device* device)
    {
        // Configure camera and get parameters for person tracking video module
        auto actualModuleConfig = ConfigureCamera(device, cfg);

        // Configure projection
        rs::core::intrinsics color_intrin = rs::utils::convert_intrinsics(device->get_stream_intrinsics(rs::stream::color));
        rs::core::intrinsics depth_intrin = rs::utils::convert_intrinsics(device->get_stream_intrinsics(rs::stream::depth));
        rs::core::extrinsics extrinsics = rs::utils::convert_extrinsics(device->get_extrinsics(rs::stream::depth, rs::stream::color));
        actualModuleConfig.projection = rs::core::projection_interface::create_instance(&color_intrin, &depth_intrin, &extrinsics);

        // Enabling Person head pose and orientation
        ptModule->QueryConfiguration()->QueryTracking()->Enable();
        ptModule->QueryConfiguration()->QueryTracking()->EnableHeadBoundingBox();


        // Set the enabled module configuration
        if(ptModule->set_module_config(actualModuleConfig) != rs::core::status_no_error)
        {
            cerr<<"error : failed to set the enabled module configuration" << endl;
            return -1;
        }

        pt_initialized = true;

        cout << "init_pt complete" << endl;

        return 0;
    }

    bool negotiate_supported_cfg(
        rs::core::video_module_interface::supported_module_config& slam_config)
    {
        slam_config.image_streams_configs[(int)rs::stream::color].size.width = 640;
        slam_config.image_streams_configs[(int)rs::stream::color].size.height = 480;
        slam_config[stream_type::color].is_enabled = true;
        slam_config[stream_type::color].frame_rate = 30.f;

        return true;
    }

    int query_supported_config(rs::device* device,
                               video_module_interface::supported_module_config& supported_config)
    {
        supported_config.image_streams_configs[(int)rs::stream::color].size.width = 640;
        supported_config.image_streams_configs[(int)rs::stream::color].size.height = 480;
        supported_config[stream_type::color].is_enabled = true;
        supported_config[stream_type::color].frame_rate = 30.f;

        supported_config.image_streams_configs[(int)rs::stream::depth].size.width = 320;
        supported_config.image_streams_configs[(int)rs::stream::depth].size.height = 240;
        supported_config[stream_type::depth].is_enabled = true;
        supported_config[stream_type::depth].frame_rate = 30.f;

        return 0;
    }

    int process_pt(rs::core::correlated_sample_set& pt_sample_set)
    {
        std::unique_lock<std::mutex> mtx_lock(mtx);

        if(!pt_initialized || pt_is_running || m_stopped)
        {
            // Drop the frame
            mtx_lock.unlock();
            return false;
        }

        pt_is_running = true;
        m_sample_set = &pt_sample_set;

        std::thread pt_work_thread(&PT_module::pt_worker, this);
        pt_work_thread.detach();

        mtx_lock.unlock();

        return 0;
    }

    void pt_worker()
    {
        // Process frame
        if (ptModule->process_sample_set(*m_sample_set) != rs::core::status_no_error)
        {
            cerr << "error : failed to process sample" << endl;
            return;
        }

        m_module_listener->on_person_tracking_finished(*m_sample_set, ptModule.get());

        pt_is_running = false;
    }

    set<int>* get_persion_ids(PersonTrackingData* trackingData)
    {
        set<int>* id_set = new set<int>;
        for (int index = 0; index < trackingData->QueryNumberOfPeople(); ++index)
        {
            PersonTrackingData::Person* personData = trackingData->QueryPersonData(
                        Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, index);

            if (personData)
            {
                PersonTrackingData::PersonTracking* personTrackingData = personData->QueryTracking();

                int id = personTrackingData->QueryId();
                id_set->insert(id);
            }
        }
        return id_set;
    }

    wstring get_data_files_path()
    {
        struct stat stat_struct;
        if(stat(PERSON_TRACKING_DATA_FILES, &stat_struct) != 0)
        {
            cerr <<  "Failed to find person tracking data files at " <<  PERSON_TRACKING_DATA_FILES << endl;
            cerr <<  "Please check that you run sample from correct directory" << endl;
            exit(EXIT_FAILURE);
        }

        string person_tracking_data_files = PERSON_TRACKING_DATA_FILES;
        int size = person_tracking_data_files.length();
        wchar_t wc_person_tracking_data_files[size + 1];
        mbstowcs(wc_person_tracking_data_files, person_tracking_data_files.c_str(), size + 1);

        return wstring(wc_person_tracking_data_files);
    }

    rs::core::video_module_interface::actual_module_config ConfigureCamera (
        rs::device* device,
        rs::core::video_module_interface::supported_module_config& cfg)
    {
        rs::core::video_module_interface::actual_module_config actualModuleConfig = {};

        // Person tracking uses only color & depth
        vector<rs::core::stream_type> possible_streams = {rs::core::stream_type::depth,
                                                          rs::core::stream_type::color
                                                         };
        for (auto &stream : possible_streams)
        {
            rs::stream librealsenseStream = rs::utils::convert_stream_type(stream);
            auto &supported_stream_config = cfg[stream];
            int width = supported_stream_config.size.width;
            int height = supported_stream_config.size.height;
            int frame_rate = supported_stream_config.frame_rate;

            rs::core::video_module_interface::actual_image_stream_config &actualStreamConfig = actualModuleConfig[stream];
            actualStreamConfig.size.width = width;
            actualStreamConfig.size.height = height;
            actualStreamConfig.frame_rate = frame_rate;
            actualStreamConfig.intrinsics = rs::utils::convert_intrinsics(
                                                device->get_stream_intrinsics(librealsenseStream));
            actualStreamConfig.extrinsics = rs::utils::convert_extrinsics(
                                                device->get_extrinsics(rs::stream::depth, librealsenseStream));
            actualStreamConfig.is_enabled = true;
        }
        return actualModuleConfig;

    }

    int8_t get_pixel_size(rs::format format)
    {
        switch(format)
        {
        case rs::format::any:
            return 0;
        case rs::format::z16:
            return 2;
        case rs::format::disparity16:
            return 2;
        case rs::format::xyz32f:
            return 4;
        case rs::format::yuyv:
            return 2;
        case rs::format::rgb8:
            return 3;
        case rs::format::bgr8:
            return 3;
        case rs::format::rgba8:
            return 4;
        case rs::format::bgra8:
            return 4;
        case rs::format::y8:
            return 1;
        case rs::format::y16:
            return 2;
        case rs::format::raw8:
            return 1;
        case rs::format::raw10:
            return 0;//not supported
        case rs::format::raw16:
            return 2;
        }
    }

    bool get_pt_running()
    {
        return  pt_is_running;
    }

    bool get_pt_is_ready()
    {
        return pt_initialized && !pt_is_running && !m_stopped;
    }

    void stop()
    {
        m_stopped = true;
    }

private:
    wstring dataDir;
    unique_ptr<rs::person_tracking::person_tracking_video_module_interface> ptModule;

    int lastPersonCount;
    int totalPersonIncrements;
    int prevPeopleInFrame;
    int prevPeopleTotal;
    set<int> *id_set;

    bool pt_initialized;
    bool pt_is_running;
    bool m_stopped;
    std::mutex mtx;

    rs::core::correlated_sample_set* m_sample_set;
    module_result_listener_interface* m_module_listener;

};
