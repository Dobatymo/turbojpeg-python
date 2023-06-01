#include <pybind11/pybind11.h>
#include <stdexcept>

extern "C" {
#include "turbojpeg.h"
}

namespace py = pybind11;

py::bytes decompress(py::bytes img, TJPF pixelformat, bool fastupsample,
                     bool fastdct) {

  tjhandle tjInstance;
  const unsigned char *jpegBuf;
  unsigned char *imgBuf;
  size_t jpegSize, imgSize;
  int result;
  int width, height, subsamp, colorspace, precision, progressive, lossless,
      xdensity, ydensity, densityunits;

  tjInstance = tj3Init(TJINIT_DECOMPRESS);
  if (tjInstance == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  std::string_view v = std::string_view(img);

  jpegBuf = reinterpret_cast<const unsigned char *>(v.data());
  jpegSize = v.size();

  result = tj3DecompressHeader(tjInstance, jpegBuf, jpegSize);
  if (result != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
  height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
  subsamp = tj3Get(tjInstance, TJPARAM_SUBSAMP);
  colorspace = tj3Get(tjInstance, TJPARAM_COLORSPACE);
  precision = tj3Get(tjInstance, TJPARAM_PRECISION);
  progressive = tj3Get(tjInstance, TJPARAM_PROGRESSIVE);
  lossless = tj3Get(tjInstance, TJPARAM_LOSSLESS);
  xdensity = tj3Get(tjInstance, TJPARAM_XDENSITY);
  ydensity = tj3Get(tjInstance, TJPARAM_YDENSITY);
  densityunits = tj3Get(tjInstance, TJPARAM_DENSITYUNITS);

  if (fastupsample) {
    if (tj3Set(tjInstance, TJPARAM_FASTUPSAMPLE, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }
  if (fastdct) {
    if (tj3Set(tjInstance, TJPARAM_FASTDCT, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  imgSize = width * height * tjPixelSize[pixelformat];
  imgBuf = static_cast<unsigned char *>(tj3Alloc(imgSize));
  if (imgBuf == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  result =
      tj3Decompress8(tjInstance, jpegBuf, jpegSize, imgBuf, 0, pixelformat);
  if (result != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  py::bytes out(std::string(reinterpret_cast<char *>(imgBuf), imgSize));
  tj3Free(imgBuf);

  tj3Destroy(tjInstance);

  return out;
}

py::bytes transform(py::bytes img, TJXOP op, int x, int y, int w, int h,
                    bool perfect, bool trim, bool crop, bool gray,
                    bool nooutput, bool progressive, bool copynone,
                    bool arithmetic, bool optimize) {

  int result;
  tjhandle tjInstance;
  const unsigned char *jpegBuf;
  size_t jpegSize;

  tjInstance = tj3Init(TJINIT_TRANSFORM);
  if (tjInstance == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  std::string_view v = std::string_view(img);

  jpegBuf = reinterpret_cast<const unsigned char *>(v.data());
  jpegSize = v.size();
  int n = 1;
  unsigned char *dstBufs[1] = {NULL};
  size_t dstSizes[1] = {0};
  tjtransform transforms[1];

  transforms[0].r = {.x = x, .y = y, .w = w, .h = h};
  transforms[0].op = op;
  transforms[0].options = 0;
  transforms[0].data = NULL;
  transforms[0].customFilter = NULL;

  if (perfect) {
    transforms[0].options |= TJXOPT_PERFECT;
  }
  if (trim) {
    transforms[0].options |= TJXOPT_TRIM;
  }
  if (crop) {
    transforms[0].options |= TJXOPT_CROP;
  }
  if (gray) {
    transforms[0].options |= TJXOPT_GRAY;
  }
  if (nooutput) {
    transforms[0].options |= TJXOPT_NOOUTPUT;
  }
  if (progressive) {
    transforms[0].options |= TJXOPT_PROGRESSIVE;
  }
  if (copynone) {
    transforms[0].options |= TJXOPT_COPYNONE;
  }
  if (arithmetic) {
    transforms[0].options |= TJXOPT_ARITHMETIC;
  }
  if (optimize) {
    transforms[0].options |= TJXOPT_OPTIMIZE;
  }

  result = tj3Transform(tjInstance, jpegBuf, jpegSize, n, dstBufs, dstSizes,
                        transforms);
  if (result != 0) {
    for (unsigned char *dstBuf : dstBufs) {
      tj3Free(dstBuf);
    }
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  py::bytes out(std::string(reinterpret_cast<char *>(dstBufs[0]), dstSizes[0]));
  for (unsigned char *dstBuf : dstBufs) {
    tj3Free(dstBuf);
  }

  tj3Destroy(tjInstance);

  return out;
}

PYBIND11_MODULE(turbojpeg, m) {

  py::enum_<TJPF>(m, "PF")
      .value("RGB", TJPF_RGB)
      .value("BGR", TJPF_BGR)
      .value("RGBX", TJPF_RGBX)
      .value("BGRX", TJPF_BGRX)
      .value("XBGR", TJPF_XBGR)
      .value("XRGB", TJPF_XRGB)
      .value("GRAY", TJPF_GRAY)
      .value("RGBA", TJPF_RGBA)
      .value("BGRA", TJPF_BGRA)
      .value("ABGR", TJPF_ABGR)
      .value("ARGB", TJPF_ARGB)
      .value("CMYK", TJPF_CMYK)
      .value("UNKNOWN", TJPF_UNKNOWN)
      .export_values();

  py::enum_<TJXOP>(m, "OP")
      .value("NONE", TJXOP_NONE)
      .value("HFLIP", TJXOP_HFLIP)
      .value("VFLIP", TJXOP_VFLIP)
      .value("TRANSPOSE", TJXOP_TRANSPOSE)
      .value("TRANSVERSE", TJXOP_TRANSVERSE)
      .value("ROT90", TJXOP_ROT90)
      .value("ROT180", TJXOP_ROT180)
      .value("ROT270", TJXOP_ROT270)
      .export_values();

  m.def("decompress", &decompress, "Decompress JPEG", py::arg("img"),
        py::arg("pixelformat") = TJPF_RGB, py::arg("fastupsample") = false,
        py::arg("fastdct") = false);

  m.def("transform", &transform, "Losslessly transform JPEG", py::arg("img"),
        py::arg("op") = TJXOP_NONE, py::arg("x") = -1, py::arg("y") = -1,
        py::arg("w") = -1, py::arg("h") = -1, py::arg("perfect") = true,
        py::arg("trim") = false, py::arg("crop") = false,
        py::arg("gray") = false, py::arg("nooutput") = false,
        py::arg("progressive") = false, py::arg("copynone") = false,
        py::arg("arithmetic") = false, py::arg("optimize") = false);
}
