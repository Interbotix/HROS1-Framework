/*
 * ColorFinder.h
 *
 *  Created on: 2011. 1. 10.
 *      Author: zerom
 */

#ifndef COLORFINDER_H_
#define COLORFINDER_H_

#include <string>

#include "Point.h"
#include "Image.h"
#include "minIni.h"

#define COLOR_SECTION   "Find Color"
#define INVALID_VALUE   -1024.0

namespace Robot
{
    class ColorFinder
    {
        private:
            Point2D m_center_point;

            void Filtering(Image* img);

        public:
            int m_hue;             /* 0 ~ 360 */
            int m_hue_tolerance;   /* 0 ~ 180 */
            int m_min_saturation;  /* 0 ~ 100 */
            int m_max_saturation;  /* 0 ~ 100 */
            int m_min_value;       /* 0 ~ 100 */
            double m_min_percent;  /* 0.0 ~ 100.0 */
            double m_max_percent;  /* 0.0 ~ 100.0 */
            double m_width_percent;/* 0.0 ~ 100.0 */
            double m_height_percent;/* 0.0 ~ 100.0 */

            std::string color_section;

            Image*  m_result;

            ColorFinder();
            ColorFinder(int hue, int hue_tol, int min_sat, int max_sat, int min_val, double min_per, double max_per);
            virtual ~ColorFinder();

            void LoadINISettings(minIni* ini);
            void LoadINISettings(minIni* ini, const std::string &section);
            void SaveINISettings(minIni* ini);
            void SaveINISettings(minIni* ini, const std::string &section);

            Point2D& GetPosition(Image* hsv_img);
    };
}

#endif /* COLORFINDER_H_ */
