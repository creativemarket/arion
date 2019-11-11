// Local
#include "arion.hpp"
#include "models/resize.hpp"
#include "carion.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct ArionResizeResult ArionResize(struct ArionInputOptions inputOptions,
                                     struct ArionResizeOptions resizeOptions) {
  struct ArionResizeResult result;

  // Unsupported format requested
  if (inputOptions.outputFormat > FORMAT_MAX) {
    result.outputData = 0;
    result.outputSize = 0;
    result.returnCode = -1;
    return result;
  }

  std::vector<unsigned char> buffer;

  Arion arion;

  std::string inputUrl = std::string(inputOptions.inputUrl);

  if (!arion.setInputUrl(inputUrl)) {
    result.outputData = 0;
    result.outputSize = 0;
    result.returnCode = -1;
    return result;
  }

  arion.addResizeOperation(resizeOptions);

  // We just passed in one operation, use the 0th index
  const int operation = 0;

  if (!arion.run()) {
    result.outputData = 0;
    result.outputSize = 0;
    result.returnCode = -1;
    return result;
  }

  if (inputOptions.outputFormat == FORMAT_JPEG) {
    if (!arion.getJpeg(operation, buffer)) {
      result.outputData = 0;
      result.outputSize = 0;
      result.returnCode = -1;
      return result;
    }
  } else if (inputOptions.outputFormat == FORMAT_PNG) {
    if (!arion.getPNG(operation, buffer)) {
      result.outputData = 0;
      result.outputSize = 0;
      result.returnCode = -1;
      return result;
    }
  }
  } else if (inputOptions.outputFormat == FORMAT_JP2) {
    if (!arion.getJpeg2k(operation, buffer)) {
      result.outputData = 0;
      result.outputSize = 0;
      result.returnCode = -1;
      return result;
    }
  }
  } else if (inputOptions.outputFormat == FORMAT_WEBP) {
    if (!arion.getWebP(operation, buffer)) {
      result.outputData = 0;
      result.outputSize = 0;
      result.returnCode = -1;
      return result;
    }
  }

  result.outputSize = buffer.size();
  result.outputData = (unsigned char *) malloc(buffer.size());

  // Get our data onto the heap
  // TODO: is there a way without this memcpy?
  memcpy(result.outputData, &buffer[0], buffer.size());

  result.returnCode = 0;

  return result;
}
