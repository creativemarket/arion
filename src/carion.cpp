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

  // Currently only two output formats are supported, JPEG (0) and PNG (1)
  if (inputOptions.outputFormat > 1) {
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

  if (inputOptions.outputFormat == 0) {
    if (!arion.getJpeg(operation, buffer)) { // JPEG
      result.outputData = 0;
      result.outputSize = 0;
      result.returnCode = -1;
      return result;
    }
  } else { // PNG
    if (!arion.getPNG(operation, buffer)) { // JPEG
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
