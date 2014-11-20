/*****************************************************************************
 * Copyright (C) 2014 Visualink
 *
 * Authors: Adrien Maglo <adrien@visualink.io>
 *
 * This file is part of Pastec.
 *
 * Pastec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pastec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Pastec.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include <iostream>
#include <vector>
#include <stdio.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <imageloader.h>
#include <messages.h>


#define STORE_SEARCHED_IMAGE
#define STORE_SEARCHED_IMAGE_TEMPLATE_NAME "cache/%s.jpg"
#define STORE_SEARCHED_IMAGE_TEMPLATE_SIZE 43

#ifdef STORE_SEARCHED_IMAGE
#include <md5/md5.h>

// Convert string to MD5 hash
// From http://answers.opencv.org/question/21035/how-to-convert-cvmat-to-stdstring-and-hash-it/
char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char digestHexadecimal[33];
    char *out = (char*)malloc(STORE_SEARCHED_IMAGE_TEMPLATE_SIZE);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(digestHexadecimal[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }
    snprintf(out, STORE_SEARCHED_IMAGE_TEMPLATE_SIZE, STORE_SEARCHED_IMAGE_TEMPLATE_NAME, digestHexadecimal);

    return out;
}
#endif

u_int32_t ImageLoader::loadImage(unsigned i_imgSize, char *p_imgData, Mat &img)
{
    vector<char> imgData(i_imgSize);
    memcpy(imgData.data(), p_imgData, i_imgSize);

    try
    {
        img = imdecode(imgData, CV_LOAD_IMAGE_GRAYSCALE);
#ifdef STORE_SEARCHED_IMAGE
        char * filename = 0;
        // Save request to cache
        filename = str2md5((char*)img.data, (int)img.step[0] * img.rows);
        cout << "Store image: " << filename << endl;
        imwrite(filename, img);
        free(filename);
#endif
    }
    catch (cv::Exception& e) // The decoding of an image can raise an exception.
    {
        const char* err_msg = e.what();
        cout << "Exception caught: " << err_msg << endl;
        return IMAGE_NOT_DECODED;
    }

    if (!img.data)
    {
        cout << "Error reading the image." << std::endl;
        return IMAGE_NOT_DECODED;
    }

    unsigned i_imgWidth = img.cols;
    unsigned i_imgHeight = img.rows;


    if (i_imgWidth > 1000
        || i_imgHeight > 1000)
    {
        cout << "Image too large, resizing." << endl;
        Size size;
        if (i_imgWidth > i_imgHeight)
        {
            size.width = 1000;
            size.height = (float)i_imgHeight / i_imgWidth * 1000;
        }
        else
        {
            size.width = (float)i_imgWidth / i_imgHeight * 1000;
            size.height = 1000;
        }
        resize(img, img, size);
        return OK;
    }

#if 1
    if (i_imgWidth < 150
        || i_imgHeight < 150)
    {
        cout << "Image too small." << endl;
        return IMAGE_SIZE_TOO_SMALL;
    }
#endif

    return OK;
}
