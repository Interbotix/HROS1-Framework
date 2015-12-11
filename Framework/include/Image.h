/*
 *   Image.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _IMAGE_H_
#define _IMAGE_H_


namespace Robot
{
	class Image
	{
		private:

		protected:

		public:
			static const int YUV_PIXEL_SIZE = 4;
			static const int RGB_PIXEL_SIZE = 3;
			static const int HSV_PIXEL_SIZE = 4;

			unsigned char *m_ImageData; /* pointer to aligned image data */
			int m_Width;                /* image width in pixels */
			int m_Height;               /* image height in pixels */
			int m_PixelSize;            /* pixel size in bytes */
			int m_NumberOfPixels;       /* number of pixels */
			int m_WidthStep;            /* size of aligned image row in bytes */
			int m_ImageSize;            /* image data size in bytes (=image->m_Height*image->m_WidthStep) */
			unsigned int *y_table;
			Image(int width, int height, int pixelsize);
			virtual ~Image();

			Image& operator = (Image &img);
	};

	class FrameBuffer
	{
		private:

		protected:

		public:
			Image *m_YUVFrame;
			Image *m_RGBFrame;
			Image *m_HSVFrame;

			FrameBuffer(int width, int height);
			virtual ~FrameBuffer();
	};
}

#endif
