//------------------------------------------------------------------------------
//
// Arion
//
// Extract metadata and create beautiful thumbnails of your images.
//
// ------------
//  arion.cpp
// ------------
//
// Copyright (c) 2015-2016 Paul Filitchkin, Snapwire
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

// Local
#include "models/operation.hpp"
#include "models/resize.hpp"
#include "utils/utils.hpp"
#include "arion.hpp"

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Lib Raw to handle raw files
#include "libraw/libraw.h"

// Stdlib
#include <iostream>
#include <string>

using namespace boost::program_options;
using namespace boost::filesystem;
using boost::property_tree::ptree;
using namespace std;

//------------------------------------------------------------------------------
// Exceptions
//------------------------------------------------------------------------------
class ArionImageExtractException : public exception {
  virtual const char *what() const throw() {
    return "Failed to extract image";
  }
} extractException;

class ArionOperationNotSupportedException : public exception {
  virtual const char *what() const throw() {
    return "Operation not supported";
  }
} operationNotSupportedException;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::Arion() :
    mInputFile(),
    mTotalOperations(0),
    mFailedOperations(0),
    mResult(false),
    mDecodeImage(true) {
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::~Arion() {
  mOperations.release();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::setSourceImage(cv::Mat &sourceImage) {
  mSourceImage = sourceImage;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::setDecodeImage(bool decodeImage) {
  mDecodeImage = decodeImage;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cv::Mat &Arion::getSourceImage() {
  return mSourceImage;
}

//------------------------------------------------------------------------------
//  Manually pass in an input URL rather than reading it from JSON
//------------------------------------------------------------------------------
bool Arion::setInputUrl(const string &inputUrl) {
  try {
    parseInputUrl(inputUrl);
  }
  catch (exception &e) {
    mResult = false;
    mErrorMessage = e.what();

    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::addResizeOperation(struct ArionResizeOptions options) {
  // This is a resize operation so create the corresponding object
  Resize *resize = new Resize();

  if (options.algo) {
    string type(options.algo);
    resize->setType(type);
  }

  resize->setHeight(options.height);
  resize->setWidth(options.width);

  if (options.interpolation) {
    string interpolation(options.interpolation);
    resize->setInterpolation(interpolation);
  }

  // Gravity
  if (options.gravity) {
    string gravity(options.gravity);
    resize->setGravity(gravity);
  }

  // Quality
  resize->setQuality(options.quality);

  // Sharpening
  resize->setSharpenAmount(options.sharpenAmount);
  resize->setSharpenRadius(options.sharpenRadius);

  // Watermark
  if (options.watermarkUrl) {
    string watermarkUrl(options.watermarkUrl);
    resize->setWatermarkUrl(watermarkUrl);
    resize->setWatermarkAmount(options.watermarkAmount);
    resize->setWatermarkMinMax(options.watermarkMin, options.watermarkMax);
  }

  if (options.watermarkType) {
    string watermarkType(options.watermarkType);
    resize->setWatermarkType(watermarkType);
  }

  // Output Url
  if (options.outputUrl) {
    string outputUrl = std::string(options.outputUrl);
    resize->setOutputUrl(outputUrl);
  }

  // Add to operation queue
  mOperations.push_back(resize);
}

//------------------------------------------------------------------------------
// Helper method for parsing the input URL
// 
// We use the input URL convention here to future proof this method.
// For instance we could have a URL that is not a local file. It could
// be a URL for another service (e.g. S3)
//------------------------------------------------------------------------------
void Arion::parseInputUrl(std::string inputUrl) {
  int pos = inputUrl.find(Utils::FILE_SOURCE);

  if (pos != string::npos) {
    mInputFile = Utils::getStringTail(inputUrl, pos + Utils::FILE_SOURCE.length());
  } else {
    // Assume it's a local file...
    mInputFile = inputUrl;
    //throw inputSourceException;
  }
}

//------------------------------------------------------------------------------
// Given each input operation do the following:
//  1. Get its type and parameters
//  2. Create the corresponding operation object and place it in the queue
//  3. Provide any additional data to the operation
//------------------------------------------------------------------------------
bool Arion::parseOperations(const ptree &pt) {
  int operationParseCount = 0;

  // Prep all operations before running them
  BOOST_FOREACH(
  const ptree::value_type &node, pt.get_child("operations"))
  {
    try {
      const ptree &operationTree = node.second;

      // "type" is not optional, throws exception if missing or unknown
      string type = operationTree.get<std::string>("type");

      // Get all of the params for this operation
      // "params" is not optional, throws exception if missing
      const ptree &paramsTree = operationTree.get_child("params");

      Operation *operation;

      if (type == "resize") {
        // This is a resize operation so create the corresponding object
        operation = new Resize();
        mDecodeImage = true;//wee need to decode image
      } else {
        throw operationNotSupportedException;
      }

      operation->setup(paramsTree);

      // Add to operation queue
      mOperations.push_back(operation);

      operationParseCount++;

    }
    catch (std::exception &e) {

      stringstream ss;

      ss << "Count not parse operation " << (operationParseCount + 1) << " - " << e.what();

      mErrorMessage = ss.str();

      return false;

    }
  }

  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::extractImageData(const string &imageFilePath) {
  // If we are taking metadata into account first read the image into memory
  // and then extract pixel and metadata from memory...
  std::ifstream input(imageFilePath.c_str(), std::ios::binary);

  // copies all data into buffer
  std::vector<char> buffer((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));

  if (buffer.empty()) {
    throw extractException;
  }

  if (mDecodeImage) {//only read pixels if required by operations
    LibRaw libRaw;
    int status = libRaw.open_buffer(static_cast<void *>(buffer.data()), buffer.size());
    if (status == LIBRAW_SUCCESS) {//only 0 is success

//   libRaw.imgdata.idata.raw_count; //TODO support multiple images
      libRaw.imgdata.params.use_camera_wb = 1;
      libRaw.unpack();// decode bayer data

      libRaw.dcraw_process();// white balance, color interpolation, color space conversion
      // gamma correction, image rotation, 3-component RGB bitmap creation
      libraw_processed_image_t *rawImage = libRaw.dcraw_make_mem_image();

      // rawImage->type; //TODO ?

      mSourceImage = cv::Mat(
          rawImage->height,
          rawImage->width,
          CV_8UC3,
          rawImage->data
      );
      cv::cvtColor(mSourceImage, mSourceImage, cv::COLOR_RGB2BGR);    //Convert RGB to BGR
    } else {
      // Now actually decode the bytes
      cv::InputArray buf(buffer);

      // decode image data without applying any exif orientation conversion and maintaining any alpha channel
      // cv::IMREAD_COLOR would otherwise drop alpha channel & change orientation if specified by exif data
      mSourceImage = cv::imdecode(buf, cv::IMREAD_UNCHANGED);

      // down-sample 16-bit images to 8-bit
      if (mSourceImage.depth() > 1) {
        mSourceImage.convertTo(mSourceImage, CV_8U, 1.0/256.0);
      }
    }

    if (mSourceImage.empty()) {
      throw extractException;
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::run() {

  //----------------------------------
  //        Preprocessing
  //----------------------------------
  if (mInputFile.length()) {
    try {
      // We have an input file, so lets read it
      extractImageData(mInputFile);
    }
    catch (boost::exception &e) {

      mResult = false;
      mErrorMessage = "Error extracting image";

      return false;
    }
    catch (std::exception &e) {
      mResult = false;
      mErrorMessage = e.what();;

      return false;
    }
  }

  // Make sure we have image data to work with
  if (mDecodeImage && mSourceImage.empty()) {
    mResult = false;
    mErrorMessage = "Input image data is empty";

    return false;
  }

  //----------------------------------
  //       Execute operations
  //----------------------------------

  mTotalOperations = mOperations.size();

  BOOST_FOREACH(Operation & operation, mOperations)
  {
    try {

      operation.setImage(mSourceImage);
      if (!operation.run()) {
        mFailedOperations++;
      }
    }
    catch (std::exception &e) {
      mFailedOperations++;
      mErrorMessage = e.what();
      return mResult;
    }
  }

  // Result of command (all operations must succeed to get true)
  if (mFailedOperations == 0) {
    mResult = true;
  } else {
    mResult = false;
  }

  return mResult;

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::getJpeg(unsigned operationIndex, std::vector<unsigned char> &data) {

  if (operationIndex >= mOperations.size()) {
    mErrorMessage = "Invalid operation to JPEG encode";

    return false;
  }

  Operation &operation = mOperations.at(operationIndex);

  bool result = operation.getJpeg(data);

  if (!result) {
    mErrorMessage = "Could not encode JPEG";
  }

  return result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::getPNG(unsigned operationIndex, std::vector<unsigned char>& data) {

  if (operationIndex >= mOperations.size()) {
    mErrorMessage = "Invalid operation to PNG encode";

     return false;
  }

   Operation& operation = mOperations.at(operationIndex);

   bool result = operation.getPNG(data);

  if (!result) {
    mErrorMessage = "Could not encode PNG";
  }

   return result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::getWebP(unsigned operationIndex, std::vector<unsigned char>& data) {

  if (operationIndex >= mOperations.size()) {
    mErrorMessage = "Invalid operation to WebP encode";

     return false;
  }

   Operation& operation = mOperations.at(operationIndex);

   bool result = operation.getWebP(data);

  if (!result) {
    mErrorMessage = "Could not encode WebP";
  }

   return result;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::getJpeg2k(unsigned operationIndex, std::vector<unsigned char>& data) {

  if (operationIndex >= mOperations.size()) {
    mErrorMessage = "Invalid operation to JPEG 2000 encode";

     return false;
  }

   Operation& operation = mOperations.at(operationIndex);

   bool result = operation.getJpeg2k(data);

  if (!result) {
    mErrorMessage = "Could not encode JPEG 2000";
  }

   return result;
}
