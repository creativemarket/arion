#ifndef RESIZE_HPP
#define RESIZE_HPP

//------------------------------------------------------------------------------
//
// Copyright (c) 2015 Paul Filitchkin, Snapwire
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//    * Neither the name of the organization nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//------------------------------------------------------------------------------

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

// OpenCV
#include <opencv2/core/core.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

enum
{
  ResizeTypeFixedWidth = 0,
  ResizeTypeFixedHeight = 1,
  ResizeTypeSquare = 2,
};

enum
{
  ResizeStatusDidNotTry = 0,
  ResizeStatusPending = 1,
  ResizeStatusSuccess = 2,
  ResizeStatusError = 3,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Resize
{
  public:

    Resize(const boost::property_tree::ptree& params);

    bool run(cv::Mat& image);

    std::string getOutputFile() const;
    bool getPreserveMeta() const;
    bool getStatus() const;
    void outputStatus(std::ostream& s, unsigned indent) const;
    void setExifData(const Exiv2::ExifData* exifData);
    void setXmpData(const Exiv2::XmpData* xmpData);

  private:

    int getAspectHeight(int resizeWidth, double aspect) const;
    int getAspectWidth(int resizeHeight, double aspect) const;

    boost::property_tree::ptree mParams;

    int mType;
    int mHeight;
    int mWidth;
    int mQuality;
    bool mPreFilter;
    int mSharpenAmount;
    float mSharpenRadius;
    bool mPreserveMeta;
    std::string mWatermarkFile;
    float mWatermarkAmount;
    std::string mOutputFile;

    double mOperationTime;

    cv::Mat mImageResized;
    cv::Mat mImageResizedFinal;

    int mStatus;
    std::string mErrorMessage;

    const Exiv2::ExifData* mpExifData;
    const Exiv2::XmpData* mpXmpData;

};

#endif // RESIZE_HPP
