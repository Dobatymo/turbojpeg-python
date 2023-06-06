#include <pybind11/pybind11.h>
#include <sstream>
#include <stdexcept>

extern "C" {
#include "turbojpeg.h"
}

namespace py = pybind11;

enum class DensityUnits : int { unknown = 0, ppi = 1, ppcm = 2 };

class TjImage {
protected:
  unsigned char *buf;
  int width, height, channels;

public:
  int xdensity, ydensity;
  TJCS colorspace;
  DensityUnits densityunits;

  TjImage() = delete;
  TjImage(const TjImage &) = delete;
  TjImage &operator=(const TjImage &) = delete;

  TjImage(TjImage &&other) noexcept
      : buf(other.buf), width(other.width), height(other.height),
        channels(other.channels), colorspace(other.colorspace),
        xdensity(other.xdensity), ydensity(other.ydensity),
        densityunits(other.densityunits) {
    other.buf = nullptr;
  }

  TjImage(unsigned char *buf, int width, int height, int channels,
          TJCS colorspace, int xdensity, int ydensity,
          DensityUnits densityunits) noexcept
      : buf(buf), width(width), height(height), channels(channels),
        colorspace(colorspace), xdensity(xdensity), ydensity(ydensity),
        densityunits(densityunits) {}

  ~TjImage() {
    if (buf != nullptr) {
      tj3Free(buf);
    }
  }

  py::buffer_info buffer() {
    int itemsize = sizeof(unsigned char);
    if (channels == 1) {
      return py::buffer_info(buf, itemsize,
                             py::format_descriptor<unsigned char>::format(), 2,
                             {height, width}, {width * itemsize, itemsize});
    } else {
      return py::buffer_info(
          buf, itemsize, py::format_descriptor<unsigned char>::format(), 3,
          {height, width, channels},
          {width * channels * itemsize, channels * itemsize, itemsize});
    }
  }
};

py::bytes compress(py::buffer img, int quality, TJSAMP subsamp, TJCS colorspace,
                   bool fastdct, bool optimize, bool progressive,
                   bool arithmetic, bool lossless, int restartblocks,
                   int restartrows, int xdensity, int ydensity,
                   DensityUnits densityunits, int width, int height,
                   TJPF pixelformat) {

  tjhandle tjInstance;
  unsigned char *imgBuf;
  unsigned char *jpegBuf = NULL;
  size_t jpegSize = 0;

  tjInstance = tj3Init(TJINIT_COMPRESS);
  if (tjInstance == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  py::buffer_info bi = img.request();

  if (bi.format != py::format_descriptor<unsigned char>::format()) {
    std::stringstream ss;
    ss << "Unsupported buffer format: " << bi.format;
    throw std::runtime_error(ss.str());
  }

  if (pixelformat == TJPF_UNKNOWN) {
    if (bi.ndim == 2) {
      pixelformat = TJPF_GRAY;
    } else if (bi.ndim == 3) {
      if (bi.shape[2] == 3) {
        pixelformat = TJPF_RGB;
      } else if (bi.shape[2] == 4) {
        pixelformat = TJPF_CMYK;
      } else {
        std::stringstream ss;
        ss << "Unsupported number of channels: " << bi.shape[2];
        throw std::runtime_error(ss.str());
      }
    } else {
      std::stringstream ss;
      ss << "Unsupported number of dimensions: " << bi.ndim;
      throw std::runtime_error(ss.str());
    }
  }

  if (bi.ndim == 1) {
    if (width <= 0 || height <= 0) {
      throw std::runtime_error("Either pass a input array with more than one "
                               "dimension or set width and height");
    }
  } else if (bi.ndim == 2) {
    height = bi.shape[0];
    width = bi.shape[1];
  } else if (bi.ndim == 3) {
    height = bi.shape[0];
    width = bi.shape[1];
  } else {
    std::stringstream ss;
    ss << "Unsupported number of dimensions: " << bi.ndim;
    throw std::runtime_error(ss.str());
  }

  imgBuf = reinterpret_cast<unsigned char *>(bi.ptr);

  if (tj3Set(tjInstance, TJPARAM_QUALITY, quality) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Set(tjInstance, TJPARAM_SUBSAMP, subsamp) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (colorspace != -1) {
    if (tj3Set(tjInstance, TJPARAM_COLORSPACE, colorspace) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (fastdct) {
    if (tj3Set(tjInstance, TJPARAM_FASTDCT, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (optimize) {
    if (tj3Set(tjInstance, TJPARAM_OPTIMIZE, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (progressive) {
    if (tj3Set(tjInstance, TJPARAM_PROGRESSIVE, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (arithmetic) {
    if (tj3Set(tjInstance, TJPARAM_ARITHMETIC, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (lossless) {
    if (tj3Set(tjInstance, TJPARAM_LOSSLESS, 1) != 0) {
      throw std::runtime_error(tj3GetErrorStr(tjInstance));
    }
  }

  if (tj3Set(tjInstance, TJPARAM_RESTARTBLOCKS, restartblocks) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Set(tjInstance, TJPARAM_RESTARTROWS, restartrows) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Set(tjInstance, TJPARAM_XDENSITY, xdensity) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Set(tjInstance, TJPARAM_YDENSITY, ydensity) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Set(tjInstance, TJPARAM_DENSITYUNITS,
             static_cast<int>(densityunits)) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  if (tj3Compress8(tjInstance, imgBuf, width, 0, height, pixelformat, &jpegBuf,
                   &jpegSize) != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  py::bytes out(std::string(reinterpret_cast<char *>(jpegBuf), jpegSize));
  tj3Free(jpegBuf);
  tj3Destroy(tjInstance);

  return out;
}

TjImage decompress(py::buffer jpeg, TJPF pixelformat, bool fastupsample,
                   bool fastdct) {

  tjhandle tjInstance;
  const unsigned char *jpegBuf;
  unsigned char *imgBuf;
  size_t jpegSize, imgSize;
  int result;
  int width, height, subsamp, precision, progressive, lossless, xdensity,
      ydensity;
  int channels;
  TJCS colorspace;
  DensityUnits densityunits;

  tjInstance = tj3Init(TJINIT_DECOMPRESS);
  if (tjInstance == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  py::buffer_info bi = jpeg.request();

  if (bi.format != py::format_descriptor<unsigned char>::format()) {
    std::stringstream ss;
    ss << "Unsupported buffer format: " << bi.format;
    throw std::runtime_error(ss.str());
  }

  jpegBuf = reinterpret_cast<unsigned char *>(bi.ptr);
  jpegSize = bi.size;

  result = tj3DecompressHeader(tjInstance, jpegBuf, jpegSize);
  if (result != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  width = tj3Get(tjInstance, TJPARAM_JPEGWIDTH);
  height = tj3Get(tjInstance, TJPARAM_JPEGHEIGHT);
  subsamp = tj3Get(tjInstance, TJPARAM_SUBSAMP);
  colorspace = static_cast<TJCS>(tj3Get(tjInstance, TJPARAM_COLORSPACE));
  precision = tj3Get(tjInstance, TJPARAM_PRECISION);
  progressive = tj3Get(tjInstance, TJPARAM_PROGRESSIVE);
  lossless = tj3Get(tjInstance, TJPARAM_LOSSLESS);
  xdensity = tj3Get(tjInstance, TJPARAM_XDENSITY);
  ydensity = tj3Get(tjInstance, TJPARAM_YDENSITY);
  densityunits =
      static_cast<DensityUnits>(tj3Get(tjInstance, TJPARAM_DENSITYUNITS));

  if (precision != 8) {
    std::stringstream ss;
    ss << "Unsupported precision: " << precision;
    throw std::runtime_error(ss.str());
  }

  if (pixelformat == TJPF_UNKNOWN) {
    if (colorspace == TJCS_RGB || colorspace == TJCS_YCbCr) {
      pixelformat = TJPF_RGB;
    } else if (colorspace == TJCS_GRAY) {
      pixelformat = TJPF_GRAY;
    } else if (colorspace == TJCS_CMYK || colorspace == TJCS_YCCK) {
      pixelformat = TJPF_CMYK;
    } else {
      std::stringstream ss;
      ss << "Unsupported colorspace: " << colorspace;
      throw std::runtime_error(ss.str());
    }
  }
  channels = tjPixelSize[pixelformat];

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

  imgSize = width * height * channels;
  imgBuf = static_cast<unsigned char *>(tj3Alloc(imgSize));
  if (imgBuf == NULL) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  result =
      tj3Decompress8(tjInstance, jpegBuf, jpegSize, imgBuf, 0, pixelformat);
  if (result != 0) {
    throw std::runtime_error(tj3GetErrorStr(tjInstance));
  }

  tj3Destroy(tjInstance);
  return TjImage(imgBuf, width, height, channels, colorspace, xdensity,
                 ydensity, densityunits);
}

py::bytes transform(py::buffer jpeg, TJXOP op, int x, int y, int w, int h,
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

  py::buffer_info bi = jpeg.request();

  if (bi.format != py::format_descriptor<unsigned char>::format()) {
    std::stringstream ss;
    ss << "Unsupported buffer format: " << bi.format;
    throw std::runtime_error(ss.str());
  }

  jpegBuf = reinterpret_cast<unsigned char *>(bi.ptr);
  jpegSize = bi.size;

  int n = 1;
  unsigned char *dstBufs[1] = {NULL};
  size_t dstSizes[1] = {0};
  tjtransform transforms[1];

  transforms[0].r = {x, y, w, h};
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

  py::enum_<TJSAMP>(m, "SAMP")
      .value("Y444", TJSAMP_444)
      .value("Y422", TJSAMP_422)
      .value("Y420", TJSAMP_420)
      .value("GRAY", TJSAMP_GRAY)
      .value("Y440", TJSAMP_440)
      .value("Y411", TJSAMP_411)
      .value("Y441", TJSAMP_441)
      .value("UNKNOWN", TJSAMP_UNKNOWN)
      .export_values();

  py::enum_<TJCS>(m, "CS")
      .value("AUTO", static_cast<TJCS>(-1))
      .value("RGB", TJCS_RGB)
      .value("YCbCr", TJCS_YCbCr)
      .value("GRAY", TJCS_GRAY)
      .value("CMYK", TJCS_CMYK)
      .value("YCCK", TJCS_YCCK)
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

  py::enum_<DensityUnits>(m, "DU")
      .value("UNKNOWN", DensityUnits::unknown)
      .value("PPI", DensityUnits::ppi)
      .value("PPCM", DensityUnits::ppcm)
      .export_values();

  py::class_<TjImage>(m, "TjImage", py::buffer_protocol())
      .def_buffer(&TjImage::buffer)
      .def_readonly("colorspace", &TjImage::colorspace)
      .def_readonly("xdensity", &TjImage::xdensity)
      .def_readonly("ydensity", &TjImage::ydensity)
      .def_readonly("densityunits", &TjImage::densityunits);

  m.def("compress", &compress, "Compress raw image", py::arg("img"),
        py::arg("quality"), py::arg("subsamp"),
        py::arg("colorspace") = static_cast<TJCS>(-1),
        py::arg("fastdct") = false, py::arg("optimize") = false,
        py::arg("progressive") = false, py::arg("arithmetic") = false,
        py::arg("lossless") = false, py::arg("restartblocks") = 0,
        py::arg("restartrows") = 0, py::arg("xdensity") = 1,
        py::arg("ydensity") = 1,
        py::arg("densityunits") = DensityUnits::unknown, py::arg("width") = -1,
        py::arg("height") = -1, py::arg("pixelformat") = TJPF_UNKNOWN);

  m.def("decompress", &decompress,
        "Decompress JPEG. If pixelformat is set to PF.UNKNOWN (the default), "
        "an appropriate one is chosen automatically.",
        py::arg("jpeg"), py::arg("pixelformat") = TJPF_UNKNOWN,
        py::arg("fastupsample") = false, py::arg("fastdct") = false);

  m.def("transform", &transform, "Losslessly transform JPEG", py::arg("jpeg"),
        py::arg("op") = TJXOP_NONE, py::arg("x") = -1, py::arg("y") = -1,
        py::arg("w") = -1, py::arg("h") = -1, py::arg("perfect") = true,
        py::arg("trim") = false, py::arg("crop") = false,
        py::arg("gray") = false, py::arg("nooutput") = false,
        py::arg("progressive") = false, py::arg("copynone") = false,
        py::arg("arithmetic") = false, py::arg("optimize") = false);
}
