// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <iomanip>
#include "console_view.h"

using namespace std;

Console_View::Console_View() : prevPeopleInFrame(-1), prevPeopleTotal(-1), lastPersonCount(0), totalPersonIncrements(0), id_set(nullptr)
{
}

Console_View::~Console_View()
{
}

void Console_View::print_person_count(rs::person_tracking::person_tracking_video_module_interface* ptModule)
{
    int numPeopleInFrame = 0;
    set<int>* new_ids=nullptr;

    new_ids = GetPersonIds(ptModule->QueryOutput());
    numPeopleInFrame = ptModule->QueryOutput()->QueryNumberOfPeople();

    if (numPeopleInFrame > lastPersonCount)
        totalPersonIncrements += (numPeopleInFrame - lastPersonCount);
    else if(numPeopleInFrame == lastPersonCount  &&  id_set != nullptr)
    {
        set<int> diff;
        set_difference(id_set->begin(), id_set->end(), new_ids->begin(), new_ids->end(), inserter(diff, diff.begin()));
        totalPersonIncrements += diff.size();
    }

    if (id_set != nullptr)
        delete id_set;
    id_set = new_ids;

    lastPersonCount = numPeopleInFrame;

    if (numPeopleInFrame != prevPeopleInFrame  ||  totalPersonIncrements != prevPeopleTotal)
    {
        cout  << left << setw(25) << "Current Frame Total"  << "Cumulative" << endl;
        cout << left << setw(25) << "--------------------" << "----------" << endl;

        cout << left << setw(25) << numPeopleInFrame << totalPersonIncrements << endl << endl;

        prevPeopleInFrame = numPeopleInFrame;
        prevPeopleTotal = totalPersonIncrements;
    }
}

void Console_View::print_headpose_person_orientation_info(rs::person_tracking::person_tracking_video_module_interface* ptModule)
{
    Intel::RealSense::PersonTracking::PersonTrackingData::Person* personData = nullptr;
    Intel::RealSense::PersonTracking::PersonTrackingData::PoseEulerAngles headAngles;
    Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking::OrientationInfo orientationInfo;
    for(int index=0; index<ptModule->QueryOutput()->QueryNumberOfPeople(); index++)
    {
        personData = ptModule->QueryOutput()->QueryPersonData(Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_ID, index);
        //print head pose (pitch, roll, yaw)
        if (personData->QueryTracking()->QueryHeadPose(headAngles))
        {
            cout << "PID: " << personData->QueryTracking()->QueryId() << ", head pose (pitch, roll, yaw) " << "(" << headAngles.pitch << ", " \
                 << headAngles.roll << ", " << headAngles.yaw << ")" << endl;
        }

        //print person orientation
        orientationInfo = personData->QueryTracking()->QueryPersonOrientation();
        if (orientationInfo.confidence > ORIENTATION_CONFIDENCE_THR) //if orientation feature is disabled, confidence should be zero
        {
            cout << " orientation: " << OrientationToString(orientationInfo.orientation) << ", confidence: " << orientationInfo.confidence << "\n";
        }
    }
}

void Console_View::print_pointing_gesture_info(rs::person_tracking::person_tracking_video_module_interface* ptModule)
{
    Intel::RealSense::PersonTracking::PersonTrackingData::Person* personData = nullptr;
    Intel::RealSense::PersonTracking::PersonTrackingData::PersonGestures* personGestures = nullptr;
    for(int index=0; index<ptModule->QueryOutput()->QueryNumberOfPeople(); index++)
    {
        personData = ptModule->QueryOutput()->QueryPersonData(Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_ID, index);
        personGestures = personData->QueryGestures();

        if (personGestures != nullptr)
        {
            if (personGestures->IsPointing() && personGestures->QueryPointingInfo().confidence > 0)
            {
                cout << "Pointing detected, PID: " << personData->QueryTracking()->QueryId() << endl;
                cout << "    Pointing: origin.x:" << personGestures->QueryPointingInfo().colorPointingData.origin.x << \
                     ", origin.y: " << personGestures->QueryPointingInfo().colorPointingData.origin.y << endl;
                cout << "    Pointing: direction.x:" << personGestures->QueryPointingInfo().colorPointingData.direction.x << \
                     ", direction.y: " << personGestures->QueryPointingInfo().colorPointingData.direction.y << endl;
            }
        }
    }
}

set<int>* Console_View::GetPersonIds(Intel::RealSense::PersonTracking::PersonTrackingData* trackingData)
{
    set<int>* id_set = new set<int>;
    for (int index = 0; index < trackingData->QueryNumberOfPeople(); ++index)
    {
        Intel::RealSense::PersonTracking::PersonTrackingData::Person* personData = trackingData->QueryPersonData(Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_ID, index);
        if (personData)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking* personTrackingData = personData->QueryTracking();

            int id = personTrackingData->QueryId();
            id_set->insert(id);
        }
    }
    return id_set;
}

std::string Console_View::OrientationToString(Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking::PersonOrientation orientation)
{
    typedef  Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking PersonTracking;
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

