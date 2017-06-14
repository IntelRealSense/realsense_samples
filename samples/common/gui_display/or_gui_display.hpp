// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <iomanip>

#include "or_data_interface.h"
#include "or_configuration_interface.h"

using namespace std;
using namespace cv;
using namespace rs::core;
using namespace rs::object_recognition;

namespace gui_display
{

class or_gui_display
{
public:
    or_gui_display()
    {
    }

    /**
         * @brief GUI initialization function. Sets default values to private parameters.
         *
         * @param[in] color_info Information of a color frame from the camera. will be the size of the final window.
         * @param[in] window_name The name of the display window.
         */
    bool initialize(rs::core::image_info color_info, cv::String window_name)
    {
        m_rect_thickness = 3;
        m_rect_line_type = 8;
        m_rect_shift = 0;
        m_text_thickness = 1;
        m_text_font = cv::FONT_HERSHEY_SIMPLEX;
        m_font_scale = 0.75;
        m_text_line_type = 5;
        m_window_name = window_name;
        cv::namedWindow(window_name,cv::WINDOW_AUTOSIZE);
        cv::moveWindow(window_name, 0, 0);
        m_image.create(color_info.height, color_info.width, CV_8UC3);
        m_image = 0;
        return true;
    }


    /**
         * @brief Sets new thickness for the drawn bounding rectangles.
         *
         * @remark The thickness values are defined by OpenCV. negative values will draw a filled rectangle.
         *
         * @param[in] new_thickness The new thickness to be set
         */
    void set_rect_thickness(int newThickness)
    {
        m_rect_thickness = newThickness;
    }
    /**
         * @brief Returns the current value of the thickness of the drawn bounding rectangles.
         *
         * @return bounding rectangle thickness value.
         */
    int get_rect_thickness()
    {
        return m_rect_thickness;
    }

    /**
         * @brief Sets new line type for the drawn bounding rectangles.
         *
         * @remark The line types values are defined by OpenCV. Please view OpenCV documentation.
         *
         * @param[in] new_line_type The new line type to be set
         */
    void set_line_type(int newLineType)
    {
        m_rect_line_type = newLineType;
    }

    /**
         * @brief Returns the current value of the line type of the drawn bounding rectangles.
         *
         * @return bounding rectangle line type value.
         */
    int get_line_type()
    {
        return m_rect_line_type;
    }

    /**
         * @brief Sets new shift for the drawn bounding rectangles.
         *
         * @remark The shift values are defined by OpenCV as number of fractional bits in the point coordinates
         *
         * @param[in] new_shift The new shift value to be set
         */
    void set_rect_shift(int newShift)
    {
        m_rect_shift = newShift;
    }

    /**
         * @brief Returns the current value of the shift of the drawn bounding rectangles.
         *
         * @return bounding rectangle shift value.
         */
    int get_rect_shift()
    {
        return m_rect_shift;
    }
    /**
         * @brief Sets new text thickness for the bounding rectangle title.
         *
         * @remark The thickness values are defined by OpenCV.
         *
         * @param[in] new_text_thickness The new shift value to be set
         */
    void set_text_thickness(int newTextThickness)
    {
        m_text_thickness = newTextThickness;
    }
    /**
         * @brief Returns the current value of the textThickness for the bounding rectangle title.
         *
         * @return Text thickness value.
         */
    int get_text_thickness()
    {
        return m_text_thickness;
    }
    /**
         * @brief Sets new text font value for the bounding rectangle title.
         *
         * @remark The text font values are defined by OpenCV.
         *
         * @param[in] new_text_font The new text font to be set
         */
    void set_text_font(int newTextFont)
    {
        m_text_font = newTextFont;
    }
    /**
         * @brief Returns the text font value for the bounding rectangle title.
         *
         * @return Text font value.
         */
    int get_text_font()
    {
        return m_text_font;
    }
    /**
         * @brief Sets new text font scale value for the bounding rectangle title.
         *
         * @remark The text font scale values are defined by OpenCV.
         *
         * @param[in] new_font_scale The new text font scale to be set
         */
    void set_font_scale(int newFontScale)
    {
        m_font_scale = newFontScale;
    }
    /**
         * @brief Returns the text font scale value for the bounding rectangle title.
         *
         * @return Text font scale value.
         */
    int get_font_scale()
    {
        return m_font_scale;
    }
    /**
         * @brief Sets new text line type value for the bounding rectangle title.
         *
         * @remark The text line type values are defined by OpenCV.
         *
         * @param[in] new_text_line_type The new text font scale to be set
         */
    void set_text_line_type(int newTextLineType)
    {
        m_text_line_type = newTextLineType;
    }
    /**
         * @brief Returns the text line type value for the bounding rectangle title.
         *
         * @return Text line type value.
         */
    int get_text_line_type()
    {
        return m_text_line_type;
    }
    /**
         * @brief Returns pointer to processed image (color image)
         *
         * @return Pointer to color image
         */
    cv::Mat get_color_cvmat()
    {
        return m_image;
    }


    /**
         * @brief set the processed image (color image)
         *
         * @param[in] Pointer to color image
         */
    void set_color_image(rs::core::image_interface *color_img)
    {
        memcpy(m_image.data,color_img->query_data(), m_image.elemSize() * m_image.total());

        //opencv display working with BGR.
        cv::cvtColor(m_image,m_image,CV_RGB2BGR);

    }


    /**
         * @brief set the processed image (color image)
         *
         * @param[in] cvmat color image
         */
    void set_color_image(cv::Mat& color_img)
    {
        m_image=color_img.clone();

    }
    /**
         * @brief Returns the window name showing the streaming video.
         *
         * @return Window name.
         */
    cv::String get_win_name()
    {
        return m_window_name;
    }

    /**
     * @brief Draws text on image
     *
     * @param[in] text The text to draw
     * @param[in] origin_point The bottom left corner of the text
     * @param[in] class_ID the id of the object class, used to determine the color
     */
    void draw_text(cv::String text, cv::Point originPoint, int classID)
    {
        // Getting the rectangle color
        cv::Scalar color = get_color(classID);

        // Getting title text size
        int baseline=0;
        cv::Size nameTextSize = cv::getTextSize(text, m_text_font, m_font_scale, m_text_thickness, &baseline);
        baseline += m_text_thickness;

        // Getting beginning point of text
        cv::Point textOrigin(originPoint.x, originPoint.y - nameTextSize.height / 2);

        // Draw the box
        int filledRectangle = -1;
        cv::Point originShifted = textOrigin + cv::Point(0, baseline);
        cv::Point opositeToOrigin = textOrigin + cv::Point(nameTextSize.width, -nameTextSize.height);
        cv::rectangle(m_image, originShifted, opositeToOrigin, color, filledRectangle, m_rect_line_type, m_rect_shift);

        // Then put the text itself
        cv::putText(m_image, text, textOrigin, m_text_font, m_font_scale, get_text_color(classID), m_text_thickness, m_text_line_type);
    }

    /**
     * @brief Draws rectangle in a color suitable to classID
     *
     * @param[in] top_left_point Top left point of the rectangle
     * @param[in] width Width of the rectangle
     * @param[in] height of the rectangle
     * @param[in] class_ID the id of the object class, used to determine the color
     */
    void draw_rect_no_text(cv::Point topLeftPoint, int width, int height, int classID)
    {
        // Translating the given data to two opposite corners of the rectangle
        cv::Point* bottomRightPoint = new cv::Point(topLeftPoint.x + width, topLeftPoint.y + height);

        // Getting the rectangle color
        cv::Scalar color = get_color(classID);

        // Drawing the rectangle
        cv::rectangle(m_image, topLeftPoint, *bottomRightPoint, color, m_rect_thickness, m_rect_line_type, m_rect_shift);
    }


    /**
     * @brief Receives rectangle parameters as represented by object recognition module and an
     * object name, then draws the rectangle with the objects name as a title.
     *
     * @param[in] name Name of the bounded object.
     * @param[in] class_ID The class ID that represents the object class. Should suit the objects name.
     * @param[in] x Relative coordinate x of top left point.
     * @param[in] y Relative coordinate y of top left point.
     * @param[in] height Relative height of the rectangle.
     * @param[in] width Relative width of the rectangle.
     *
     * @return True if rectangle was drawn successfully , false otherwise.
     */
    bool draw_rect(cv::String name, int classID, int x, int y, int width, int height)
    {
        if (x ==0 || y == 0)
        {
            return false;
        }
        // Getting title text size
        int baseline=0;
        cv::Size nameTextSize = cv::getTextSize(name, m_text_font, m_font_scale, m_text_thickness, &baseline);

        int minCoordinateValue = 10;
        cv::Point* topLeftPoint = new cv::Point(std::max(minCoordinateValue,x),
                                                std::max(minCoordinateValue  + nameTextSize.height,y));

        draw_rect_no_text(*topLeftPoint, width + std::min(x,0), height  + std::min(y,0), classID);

        draw_text(name, *topLeftPoint, classID);

        return true;
    }

    /**
     * @brief Shows "No Results" message, meaning the OR localization algorithm returned no recognised results.
     */
    void draw_no_results()
    {
        cv::String text = "No object was found";
        int baseline=0;

        // Getting the rectangle color
        int classID = 0; // This text doesn't belong to any object class
        cv::Scalar color = get_color(classID); // Meaning no color - black

        //Getting title text size
        cv::Size textSize = cv::getTextSize(text, m_text_font, m_font_scale, m_text_thickness, &baseline);
        baseline += m_text_thickness;

        // Getting beginning point of text
        cv::Point textOrigin(0, 0);

        // Draw the box
        int filledRectangle = -1;
        int rectangleHeightFactor = 2; // I can't say why, but this number seems to put the rectangle in it's right place
        cv::Point originShifted = textOrigin + cv::Point(0, baseline);
        cv::Point opositeToOrigin = textOrigin + cv::Point(textSize.width, rectangleHeightFactor * textSize.height);
        cv::rectangle(m_image, originShifted, opositeToOrigin, color, filledRectangle, m_rect_line_type, m_rect_shift);

        int finalTextShift = 3; // I can't say why, but this number seems to put the text in it's right place

        // Then put the text itself
        cv::putText(m_image, text, finalTextShift * originShifted, m_text_font, m_font_scale,
                    get_text_color(classID), m_text_thickness, m_text_line_type);

    }

    /**
     * @brief Draws all localized objects upon the image, while mentioning the objects name and the
     * probability it was recognized with.
     *
     * @param[in] localization_data The output of the OR Localization process.
     * @param[in] array_size The size of the localization_data array (number of localized objects).
     * @param[in] or_configuration The configuration of the OR Localization process.
     *
     * @return True if rectangle was drawn successfully , false otherwise.
     */
    bool draw_results(rs::object_recognition::localization_data* localizationData,
                      int arraySize,
                      rs::object_recognition::or_configuration_interface* orConfiguration)
    {
        if (arraySize == 0)
        {
            draw_no_results();
            return false;
        }
        for (int i = 0; i< arraySize; i++)
        {
            cv::String title = "";
            cv::String objectName = orConfiguration->query_object_name_by_id(localizationData[i].label);

            title = objectName + ": " + std::to_string((int)(localizationData[i].probability * 100)) + "%" +
                    " " + get_3D_location_string(localizationData[i].object_center.coordinates);

            this->draw_rect(title, localizationData[i].label,
                            localizationData[i].roi.x,localizationData[i].roi.y,
                            localizationData[i].roi.width,localizationData[i].roi.height);
        }
        return true;
    }
    /**
     * @brief Draws all recognized objects upon the image, while mentioning the objects name and the
     * probability it was recognized with.
     *
     * @param[in] recognition_data The output of the OR recognition process.
     * @param[in] array_size The size of the recognition_data array (number of recognized classes).
     * @param[in] or_configuration The configuration of the OR Recognition process.
     */
    bool draw_results(rs::object_recognition::recognition_data* recognitionData,
                      int arraySize,
                      rs::object_recognition::or_configuration_interface* orConfiguration)
    {

        if (arraySize == 0)
        {
            draw_no_results();
        }

        int mostProbableObject = 0;
        cv::String title = "";
        cv::String objectName = orConfiguration->query_object_name_by_id(recognitionData[mostProbableObject].label);
        title = objectName + ": " + std::to_string((int)(recognitionData[mostProbableObject].probability * 100)) + "%";

        rs::core::rect roi = orConfiguration->query_roi();

        this->draw_rect(title, recognitionData[mostProbableObject].label, roi.x, roi.y, roi.width, roi.height);

        return true;
    }


    /**
     * @brief Draws all localized and tracked objects upon the image, while mentioning the objects name
     * and the probability it was recognized with.
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
    bool draw_results(rs::object_recognition::localization_data* localizationData,
                      rs::object_recognition::tracking_data* trackingData,
                      rs::object_recognition::or_configuration_interface* orConfiguration,
                      int arraySize,
                      bool isLocalize,
                      std::vector<std::string>& objectsNames)
    {

        if (isLocalize)
        {
            if (!draw_results(localizationData, arraySize, orConfiguration))
            {
                return false;
            }

            m_objects_IDs_for_tracking_localization.empty();

            for (int i = 0 ; i < arraySize ; i++)
            {
                std::string objectName = orConfiguration->query_object_name_by_id(localizationData[i].label);
                objectsNames.emplace_back(objectName);
                m_objects_IDs_for_tracking_localization.emplace_back(localizationData[i].label);
            }

        }
        else
        {
            for (int i = 0; i< arraySize; i++)
            {
                cv::String title = objectsNames[i] + " " + get_3D_location_string(trackingData[i].object_center.coordinates);
                this->draw_rect(title, m_objects_IDs_for_tracking_localization[i],
                                trackingData[i].roi.x,trackingData[i].roi.y,
                                trackingData[i].roi.width, trackingData[i].roi.height);
            }
        }

        return true;
    }



    /**
     * @brief Draws all tracked objects upon the image.
     *
     *
     * @param[in] tracking_data The output of the OR Tracking process.
     * @param[in] array_size The size of the localization_data array (number of localized objects).
     */
    bool draw_results(rs::object_recognition::tracking_data* trackingData,
                      int arraySize)
    {

        for (int i = 0; i< arraySize; i++)
        {
            cv::String title = get_3D_location_string(trackingData[i].object_center.coordinates);
            this->draw_rect(title, i,
                            trackingData[i].roi.x,trackingData[i].roi.y,
                            trackingData[i].roi.width, trackingData[i].roi.height);
        }

        return true;
    }



    /**
     * @brief Draws the previously calculated results
     *
     * @return Pressed key, if exists.
     */
    char show_results()
    {
        cv::imshow(m_window_name, m_image);
        return cv::waitKey(1);
    }
    /**
     * @brief Gets an object ID and returns unique color.
     *
     * @warning Addresses only objects recognized by the localization or recognition mechanism.
     * @remark In case of unrecognized object - the color black will be returned.
     *
     * @param[in] class_ID: The class ID that represents the object class.
     *
     * @return Unique RGBA color represented as a cv::Scalar.
     */
    cv::Scalar get_color(int classID)
    {
        if (classID > m_max_classes || classID < 1)
        {
            classID = m_max_classes;
        }

        return cv::Scalar(m_color_mat.at<unsigned char>(classID - 1, 0),
                          m_color_mat.at<unsigned char>(classID - 1, 1),
                          m_color_mat.at<unsigned char>(classID - 1, 2),
                          m_color_mat.at<unsigned char>(classID - 1, 3));
    }
    /**
     * @brief Gets an object ID and returns a text color that suits its unique color.
     *
     * @warning Addresses only objects recognized by the localization or recognition mechanism.
     * @remark In case of unrecognized object - the color white will be returned.
     *
     * @param[in] class_ID: The class ID that represents the object class.
     *
     * @return RGBA text color that suits the objects unique color.
     */
    cv::Scalar get_text_color(int classID)
    {
        if (classID > m_max_classes || classID < 1)
        {
            classID = m_max_classes;
        }

        return cv::Scalar(m_text_mat.at<unsigned char>(classID - 1, 0),
                          m_text_mat.at<unsigned char>(classID - 1, 1),
                          m_text_mat.at<unsigned char>(classID - 1, 2),
                          m_text_mat.at<unsigned char>(classID - 1, 3));
    }
    /**
     * @brief Gets the 3D location as a string.
     *
     * @param[in] location: A point in 3D, given in millimeters
     * @return The formatted string.
     */
    std::string get_3D_location_string(const rs::core::point3dF32& location) const
    {
        std::stringstream str_stream;
        str_stream << std::fixed << std::setprecision(1) << (location.z / 1000);
        std::string str = "@";
        str += str_stream.str() + "m";
        return str;
    }
protected:
    int m_rect_thickness;
    int m_rect_line_type;
    int m_rect_shift;
    int m_text_thickness;
    int m_text_font;
    double m_font_scale;
    int m_text_line_type;
    cv::Mat m_image;
    cv::String m_window_name;
    std::vector<int> m_objects_IDs_for_tracking_localization;

    static const int m_max_classes = 44;

    const unsigned char m_color_arr[m_max_classes][4] =
    {
        {102,0,0,0},
        {153,0,0,0},
        {255,102,102,0},
        {255,128,0,0},
        {255,178,102,0},
        {255,255,0,0},
        {76,153,0,0},
        {128,255,0,0},
        {0,255,128,0},
        {0,153,153,0},
        {0,204,204,0},
        {0,255,255,0},
        {0,76,153,0},
        {0,128,255,0},
        {153,204,255,0},
        {0,0,153,0},
        {0,0,255,0},
        {102,102,255,0},
        {76,0,153,0},
        {127,0,255,0},
        {178,102,255,0},
        {153,0,153,0},
        {204,0,204,0},
        {255,102,255,0},
        {102,0,51,0},
        {204,0,102,0},
        {255,102,178,0},
        {96,96,96,0},
        {192,192,192,0},
        {204,204,255,0},
        {255,204,229,0},
        {0,102,51,0},
        {102,51,0,0},
        {102,102,0,0},
        {153,153,0,0},
        {204,255,204,0},
        {255,255,255,0},
        {255,204,204,0},
        {153,153,255,0},
        {255,153,204,0},
        {0,153,76,0},
        {204,102,0,0},
        {0,102,102,0},
        {0,0,0,0}
    };

    const unsigned char m_text_arr[m_max_classes][4] =
    {
        {255,255,255,0},
        {255,255,255,0},
        {0,0,0,0},//3
        {255,255,255,0},
        {0,0,0,0},//5
        {0,0,0,0},
        {255,255,255,0},//7
        {0,0,0,0},
        {0,0,0,0},
        {255,255,255,0},//10
        {0,0,0,0},
        {0,0,0,0},
        {255,255,255,0},//13
        {255,255,255,0},//14
        {0,0,0,0},
        {255,255,255,0},//16
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},//28
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {255,255,255,0},//32
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},//35
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0},
        {255,255,255,0}
    };
    const cv::Mat m_color_mat = cv::Mat(m_max_classes, 4, CV_8UC1, const_cast<void*> ( (const void*) ( &m_color_arr) ));
    const cv::Mat m_text_mat = cv::Mat(m_max_classes, 4, CV_8UC1, const_cast<void*> ( (const void*) ( &m_text_arr) ));
};

std::unique_ptr<gui_display::or_gui_display> make_gui_or_display()
{
    return unique_ptr<or_gui_display> { new gui_display::or_gui_display() };
}
}






