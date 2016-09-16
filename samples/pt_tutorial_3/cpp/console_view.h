// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include "camera_utils.h"
#include <set>

extern "C"
{
    class Console_View
    {
    public:
        Console_View();
        ~Console_View();

        void print_person_count(rs::person_tracking::person_tracking_video_module_interface* ptModule);
        void print_headpose_person_orientation_info(rs::person_tracking::person_tracking_video_module_interface* ptModule);
        void print_pointing_gesture_info(rs::person_tracking::person_tracking_video_module_interface* ptModule);
        std::set<int>* GetPersonIds(Intel::RealSense::PersonTracking::PersonTrackingData* trackingData);
        std::string OrientationToString(Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking::PersonOrientation orientation);
    protected:
        std::set<int> *id_set;
        int prevPeopleInFrame;
        int prevPeopleTotal;
        int lastPersonCount;
        int totalPersonIncrements;

        static constexpr float ORIENTATION_CONFIDENCE_THR = 1.0f;
    };
}
