#ifndef CARION_H_
#define CARION_H_

#ifdef __cplusplus
extern "C" {
#endif

struct ArionInputOptions {
  // If set to 0 the image orientation will not be corrected
  // (based on the EXIF orientation flag)
  unsigned correctOrientation;

  // The location of the input image
  char *inputUrl;

  // If an output URL is provided the image will be saved there
  char *outputUrl;

  // The desired output format. 0 = JPEG, 1 = PNG
  unsigned outputFormat;
};

struct ArionResizeOptions {
  char *algo;
  unsigned height;
  unsigned width;
  char *interpolation;
  char *gravity;
  unsigned quality;
  unsigned sharpenAmount;
  float sharpenRadius;
  unsigned preserveMeta;
  char *watermarkUrl;
  char *watermarkType;
  float watermarkAmount;
  float watermarkMin;
  float watermarkMax;
  char *outputUrl;
};

struct ArionResizeResult {

  // The JPG encoded image bytes
  unsigned char *outputData;

  // The size of the JPEG encoded image bytes
  int outputSize;

  // The result of the operation
  // 0 - success
  // -1 - failure
  int returnCode;

};

struct ArionResizeResult ArionResize(struct ArionInputOptions inputOptions,
                                     struct ArionResizeOptions resizeOptions);

#ifdef __cplusplus
}
#endif

#endif