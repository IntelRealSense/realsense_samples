// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <set>
#include <iostream>
#include <iomanip>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace rs::person_tracking;

namespace console_display
{
class pt_console_display
{
public:
    pt_console_display()
    {
        // Initialize input variables
        prevPeopleInFrame = -1;
        prevPeopleTotal = -1;
        lastPersonCount = 0;
        totalPersonIncrements = 0;
        id_set = nullptr;
        need_create_window = true;

        prevX = 0.0;
        prevY = 0.0;
        prevZ = 0.0;
    }

    ~pt_console_display() {}

    int on_person_count_update(person_tracking_video_module_interface *ptModule)
    {
        int numPeopleInFrame = 0;
        set<int> *new_ids = nullptr;

        new_ids = get_person_ids(ptModule->QueryOutput());
        numPeopleInFrame = ptModule->QueryOutput()->QueryNumberOfPeople();

        if (numPeopleInFrame > lastPersonCount)
            totalPersonIncrements += (numPeopleInFrame - lastPersonCount);
        else if (numPeopleInFrame == lastPersonCount && id_set != nullptr)
        {
            set<int> diff;
            set_difference(id_set->begin(), id_set->end(), new_ids->begin(), new_ids->end(),
                           inserter(diff, diff.begin()));
            totalPersonIncrements += diff.size();
        }

        if (id_set != nullptr)
            delete id_set;
        id_set = new_ids;

        lastPersonCount = numPeopleInFrame;

        if (numPeopleInFrame != prevPeopleInFrame || totalPersonIncrements != prevPeopleTotal)
        {
            cout << left << setw(25) << "Current Frame Total" << "Cumulative" << endl;
            cout << left << setw(25) << "--------------------" << "----------" << endl;

            cout << left << setw(25) << numPeopleInFrame << totalPersonIncrements << endl << endl;

            prevPeopleInFrame = numPeopleInFrame;
            prevPeopleTotal = totalPersonIncrements;
        }

        return totalPersonIncrements;
    }

    void on_person_headpose_orientation_update(person_tracking_video_module_interface *ptModule)
    {
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->QueryNumberOfPeople() > 0)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PoseEulerAngles headAngles;
                // Print head pose (pitch, roll, yaw)
                if (personData->QueryFace()->QueryHeadPose(headAngles))
                {
                    // Ignore minor difference to avoid print overflow
                    if (abs(prevX - headAngles.pitch) > 5.0 || abs(prevY - headAngles.roll) > 4.0 ||
                            abs(prevZ - headAngles.yaw) > 5.0)
                    {
                        cout << "PID: " << personData->QueryTracking()->QueryId()
                             << ", head pose (pitch, roll, yaw) " << "(" << headAngles.pitch << ", " \
                             << headAngles.roll << ", " << headAngles.yaw << ")" << endl;

                        prevX = headAngles.pitch;
                        prevY = headAngles.roll;
                        prevZ = headAngles.yaw;
                    }
                }

            }
        }
    }

    void on_person_pointing_gesture_info_update(person_tracking_video_module_interface *ptModule)
    {
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->QueryNumberOfPeople() > 0)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = nullptr;
            personData = trackingData->QueryPersonData(
                             Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);

            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PersonGestures *personGestures = personData->QueryGestures();
                if (personGestures)
                {
                    if (personGestures->IsPointing() && personGestures->QueryPointingInfo().confidence > 0)
                    {
                        float WorldDirectionX = personGestures->QueryPointingInfo().worldPointingData.direction.x;
                        float WorldDirectionY = personGestures->QueryPointingInfo().worldPointingData.direction.y;
                        float WorldDirectionZ = personGestures->QueryPointingInfo().worldPointingData.direction.z;

                        // Ignore minor difference to avoid print overflow
                        if (abs(prevX - WorldDirectionX) > 0.05 || abs(prevY - WorldDirectionY) > 0.05 ||
                                abs(prevZ - WorldDirectionZ) > 0.05)
                        {
                            cout << "Pointing detected, PID: " << personData->QueryTracking()->QueryId() << endl;
                            cout << "    color coordinates: origin(x,y): "
                                 << personGestures->QueryPointingInfo().colorPointingData.origin.x << \
                                 ", " << personGestures->QueryPointingInfo().colorPointingData.origin.y << endl;
                            cout << "                    direction(x,y): "
                                 << personGestures->QueryPointingInfo().colorPointingData.direction.x << \
                                 ", " << personGestures->QueryPointingInfo().colorPointingData.direction.y << endl;
                            cout << "    world coordinates: origin(x,y,z): "
                                 << personGestures->QueryPointingInfo().worldPointingData.origin.x << \
                                 ", " << personGestures->QueryPointingInfo().worldPointingData.origin.y << ", "
                                 << personGestures->QueryPointingInfo().worldPointingData.origin.z << endl;
                            cout << "                    direction(x,y,z): " << WorldDirectionX << ", "
                                 << WorldDirectionY << ", " << WorldDirectionZ << endl;

                            prevX = WorldDirectionX;
                            prevY = WorldDirectionY;
                            prevZ = WorldDirectionZ;
                        }
                    }
                }
            }
        }
    }

    void set_tracking(person_tracking_video_module_interface *ptModule)
    {
        Intel::RealSense::PersonTracking::PersonTrackingData *trackingData = ptModule->QueryOutput();

        if (trackingData->GetTrackingState() ==
                Intel::RealSense::PersonTracking::PersonTrackingData::TrackingState::TRACKING_STATE_DETECTING &&
                trackingData->QueryNumberOfPeople() > 0)
        {
            // Start tracking the first person detected in the frame
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = trackingData->QueryPersonData(
                        Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, 0);
            if (personData)
            {
                cout << "Call StartTracking()" << endl;
                trackingData->StartTracking(personData->QueryTracking()->QueryId());
            }
        }
    }

    std::set<int> *get_person_ids(Intel::RealSense::PersonTracking::PersonTrackingData *trackingData)
    {
        set<int> *id_set = new set<int>;
        for (int index = 0; index < trackingData->QueryNumberOfPeople(); ++index)
        {
            Intel::RealSense::PersonTracking::PersonTrackingData::Person *personData = trackingData->QueryPersonData(
                        Intel::RealSense::PersonTracking::PersonTrackingData::ACCESS_ORDER_BY_INDEX, index);
            if (personData)
            {
                Intel::RealSense::PersonTracking::PersonTrackingData::PersonTracking *personTrackingData = personData->QueryTracking();

                int id = personTrackingData->QueryId();
                id_set->insert(id);
            }
        }
        return id_set;
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

    void render_color_frames(rs::core::image_interface *colorImage)
    {
        cv::Mat renderImage = Image2Mat(colorImage);

        if(need_create_window)
        {
            cv::namedWindow("Color image", CV_WINDOW_AUTOSIZE);
            need_create_window = false;
        }
        cv::imshow("Color image", renderImage);
        cv::waitKey(1);
    }

    void on_person_info_update(rs::person_tracking::person_tracking_video_module_interface *ptModule)
    {
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

                int pid = personTrackingData->QueryId();
                int x = box.rect.x;
                int y = box.rect.y;
                int w = box.rect.w;
                int h = box.rect.h;

                float centerMassWorldX = centerMass.world.point.x;
                float centerMassWorldY = centerMass.world.point.y;
                float centerMassWorldZ = centerMass.world.point.z;


                std::stringstream centerText;
                centerText.precision(4);
                centerText << "(" << centerMassWorldX << "," << centerMassWorldY << "," << centerMassWorldZ << ")";


                if (prevPid != pid)
                {
                    // Print out PT result
                    cout << left << setw(18) << "Person ID" << setw(28) << "Person Center" << "2D Coordinates"
                         << endl;
                    cout << left << setw(18) << "---------" << setw(28) << "-------------" << "--------------"
                         << endl;
                    cout << left << setw(18) << pid << setw(28) << centerText.str() << "(" << x << "," << y << ")"
                         << " (" << x + w << "," << y + h << ")" << endl;
                    cout << endl;

                    prevPid = pid;
                }

            }
        }
    }

protected:
    std::set<int> *id_set;
    int prevPeopleInFrame;
    int prevPeopleTotal;
    int lastPersonCount;
    int totalPersonIncrements;

    int prevPid;
    float prevX, prevY, prevZ;
    bool need_create_window;

    static constexpr float ORIENTATION_CONFIDENCE_THR = 1.0f;
};

std::unique_ptr<console_display::pt_console_display> make_console_pt_display()
{
    return unique_ptr<pt_console_display> { new console_display::pt_console_display() };
}
} // end namespace console_display
