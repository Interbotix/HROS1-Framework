/*
 *   ImgProcess.cpp
 *
 *   Author: ROBOTIS
 *
 */

#include <string.h>

#include "ImgProcess.h"

using namespace Robot;

void ImgProcess::YUVtoRGB(FrameBuffer *buf)
{
  unsigned char *yuyv, *rgb;
  int z = 0;

  yuyv = buf->m_YUVFrame->m_ImageData;
  rgb = buf->m_RGBFrame->m_ImageData;

  for (int height = 0; height < buf->m_YUVFrame->m_Height; height++)
    {
      for (int width = 0; width < buf->m_YUVFrame->m_Width; width++)
        {
          int r, g, b;
          int y, u, v;

          if (!z)
            y = yuyv[0] << 8;
          else
            y = yuyv[2] << 8;
          u = yuyv[1] - 128;
          v = yuyv[3] - 128;

          r = (y + (359 * v)) >> 8;
          g = (y - (88 * u) - (183 * v)) >> 8;
          b = (y + (454 * u)) >> 8;

          *(rgb++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
          *(rgb++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
          *(rgb++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

          if (z++)
            {
              z = 0;
              yuyv += 4;
            }
        }
    }
}

void ImgProcess::RGBtoHSV(FrameBuffer *buf)
{
  int ir, ig, ib, imin, imax;
  int th, ts, tv, diffvmin;
  unsigned char *rgb, *hsv;
  rgb = buf->m_RGBFrame->m_ImageData;
  hsv = buf->m_HSVFrame->m_ImageData;
  for (int i = 0; i < buf->m_RGBFrame->m_Width * buf->m_RGBFrame->m_Height; i++)
    {
      ir = *rgb++;
      ig = *rgb++;
      ib = *rgb++;

      if ( ir > ig )
        {
          imax = ir;
          imin = ig;
        }
      else
        {
          imax = ig;
          imin = ir;
        }

      if ( imax > ib )
        {
          if ( imin > ib ) imin = ib;
        }
      else imax = ib;

      tv = imax;
      diffvmin = imax - imin;

      if ( (tv != 0) && (diffvmin != 0) )
        {
          ts = (255 * diffvmin) / imax;
          if ( tv == ir ) th = (ig - ib) * 60 / diffvmin;
          else if ( tv == ig ) th = 120 + (ib - ir) * 60 / diffvmin;
          else th = 240 + (ir - ig) * 60 / diffvmin;
          if ( th < 0 ) th += 360;
          th &= 0x0000FFFF;
        }
      else
        {
          tv = 0;
          ts = 0;
          th = 0xFFFF;
        }

      ts = ts * 100 / 255;
      tv = tv * 100 / 255;

      //buf->m_HSVFrame->m_ImageData[i]= (unsigned int)th | ((unsigned int)ts<<16) | ((unsigned int)tv<<24);
      *hsv++ = (unsigned char)(th >> 8);
      *hsv++ = (unsigned char)(th & 0xFF);
      *hsv++ = (unsigned char)(ts & 0xFF);
      *hsv++ = (unsigned char)(tv & 0xFF);
    }
}

void ImgProcess::Erosion(Image* img)
{
  int x, y, z, index, ytable[img->m_Height];

  unsigned char* temp_img = new unsigned char[img->m_Width * img->m_Height];
  memset(temp_img, 0, img->m_Width * img->m_Height);

  index = img->m_Width;
  for (y = 0, z = 0; y < img->m_Height; y++)
    {
      ytable[y] = z;
      z += img->m_Width;
    }

  for ( y = 1; y < (img->m_Height - 1); y++ )
    {
      for ( x = 1; x < (img->m_Width - 1); x++ )
        {
          temp_img[index++] = img->m_ImageData[ytable[y - 1] + (x - 1)]
                              & img->m_ImageData[ytable[y - 1] + (x  )]
                              & img->m_ImageData[ytable[y - 1] + (x + 1)]
                              & img->m_ImageData[ytable[y] + (x - 1)]
                              & img->m_ImageData[ytable[y] + (x  )]
                              & img->m_ImageData[ytable[y] + (x + 1)]
                              & img->m_ImageData[ytable[y + 1] + (x - 1)]
                              & img->m_ImageData[ytable[y + 1] + (x  )]
                              & img->m_ImageData[ytable[y + 1] + (x + 1)];
        }
      index += 2;
    }

  memcpy(img->m_ImageData, temp_img, img->m_Width * img->m_Height);

  delete[] temp_img;
}

void ImgProcess::Erosion(Image* src, Image* dest)
{
  int x, y, z, index, ytable[src->m_Height];

  index = src->m_Width;
  for (y = 0, z = 0; y < src->m_Height; y++)
    {
      ytable[y] = z;
      z += src->m_Width;
    }
  for ( y = 1; y < (src->m_Height - 1); y++ )
    {
      for ( x = 1; x < (src->m_Width - 1); x++ )
        {
          dest->m_ImageData[index++] = src->m_ImageData[ytable[y - 1] + (x - 1)]
                                       & src->m_ImageData[ytable[y - 1] + (x  )]
                                       & src->m_ImageData[ytable[y - 1] + (x + 1)]
                                       & src->m_ImageData[ytable[y] + (x - 1)]
                                       & src->m_ImageData[ytable[y] + (x  )]
                                       & src->m_ImageData[ytable[y] + (x + 1)]
                                       & src->m_ImageData[ytable[y + 1] + (x - 1)]
                                       & src->m_ImageData[ytable[y + 1] + (x  )]
                                       & src->m_ImageData[ytable[y + 1] + (x + 1)];
        }
      index += 2;
    }
}

void ImgProcess::Dilation(Image* img)
{
  int x, y, z, index, ytable[img->m_Height];

  unsigned char* temp_img = new unsigned char[img->m_Width * img->m_Height];
  memset(temp_img, 0, img->m_Width * img->m_Height);

  index = img->m_Width;
  for (y = 0, z = 0; y < img->m_Height; y++)
    {
      ytable[y] = z;
      z += img->m_Width;
    }

  for ( y = 1; y < (img->m_Height - 1); y++ )
    {
      for ( x = 1; x < (img->m_Width - 1); x++ )
        {
          temp_img[index++] = img->m_ImageData[ytable[y - 1] + (x - 1)]
                              | img->m_ImageData[ytable[y - 1] + (x  )]
                              | img->m_ImageData[ytable[y - 1] + (x + 1)]
                              | img->m_ImageData[ytable[y] + (x - 1)]
                              | img->m_ImageData[ytable[y] + (x  )]
                              | img->m_ImageData[ytable[y] + (x + 1)]
                              | img->m_ImageData[ytable[y + 1] + (x - 1)]
                              | img->m_ImageData[ytable[y + 1] + (x  )]
                              | img->m_ImageData[ytable[y + 1] + (x + 1)];
        }
      index += 2;
    }

  memcpy(img->m_ImageData, temp_img, img->m_Width * img->m_Height);

  delete[] temp_img;
}

void ImgProcess::Dilation(Image* src, Image* dest)
{
  int x, y, z, index, ytable[src->m_Height];

  index = src->m_Width;
  for (y = 0, z = 0; y < src->m_Height; y++)
    {
      ytable[y] = z;
      z += src->m_Width;
    }
  for ( y = 1; y < (src->m_Height - 1); y++ )
    {
      for ( x = 1; x < (src->m_Width - 1); x++ )
        {
          dest->m_ImageData[index++] = src->m_ImageData[ytable[y - 1] + (x - 1)]
                                       | src->m_ImageData[ytable[y - 1] + (x  )]
                                       | src->m_ImageData[ytable[y - 1] + (x + 1)]
                                       | src->m_ImageData[ytable[y] + (x - 1)]
                                       | src->m_ImageData[ytable[y] + (x  )]
                                       | src->m_ImageData[ytable[y] + (x + 1)]
                                       | src->m_ImageData[ytable[y + 1] + (x - 1)]
                                       | src->m_ImageData[ytable[y + 1] + (x  )]
                                       | src->m_ImageData[ytable[y + 1] + (x + 1)];
        }
      index += 2;
    }
}

void ImgProcess::HFlipYUV(Image* img)
{
  int sizeline = img->m_Width * 2; /* 2 bytes per pixel*/
  unsigned char *pframe;
  pframe = img->m_ImageData;
  unsigned char line[sizeline];/*line buffer*/
  for (int h = 0; h < img->m_Height; h++)
    {
      /*line iterator*/
      for (int w = sizeline - 1; w > 0; w = w - 4)
        {
          /* pixel iterator */
          line[w - 1] = *pframe++;
          line[w - 2] = *pframe++;
          line[w - 3] = *pframe++;
          line[w] = *pframe++;
        }
      memcpy(img->m_ImageData + (h * sizeline), line, sizeline); /*copy reversed line to frame buffer*/
    }
}

void ImgProcess::VFlipYUV(Image* img)
{
  int sizeline = img->m_Width * 2; /* 2 bytes per pixel */
//    unsigned char *pframe;
//    pframe=img->m_ImageData;
  unsigned char line1[sizeline];/*line1 buffer*/
  unsigned char line2[sizeline];/*line2 buffer*/
  for (int h = 0; h < img->m_Height / 2; h++)
    {
      /*line iterator*/
      memcpy(line1, img->m_ImageData + h * sizeline, sizeline);
      memcpy(line2, img->m_ImageData + (img->m_Height - 1 - h)*sizeline, sizeline);

      memcpy(img->m_ImageData + h * sizeline, line2, sizeline);
      memcpy(img->m_ImageData + (img->m_Height - 1 - h)*sizeline, line1, sizeline);
    }
}

void ImgProcess::HVFlipYUV(Image* img)
{
  int sizeline = img->m_Width * 2; /* 2 bytes per pixel*/
  unsigned char b1, b2, t1, t2;
  unsigned char *tframe, *bframe;
  bframe = (unsigned char *)img->m_ImageData;
  tframe = &bframe[((img->m_Height - 1) * sizeline) - 1];
  for (int h = 0; h < img->m_Height / 2; h++)
    {
      for (int z = 0, w = sizeline - 1; z < sizeline / 2; z++, w--)
        {
          b1 = bframe[z];
          b2 = bframe[w];
          t1 = tframe[z];
          t2 = tframe[w];
          tframe[z] = b2;
          tframe[w] = b1;
          bframe[z] = t2;
          bframe[w] = t1;
        }
      bframe += sizeline;
      tframe -= sizeline;
    }
}

