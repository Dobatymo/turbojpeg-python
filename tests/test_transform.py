import unittest
from io import BytesIO
from pathlib import Path
from typing import List

import numpy as np
from PIL import Image
from skimage.metrics import structural_similarity

import turbojpeg
from turbojpeg import (CS, DU, OP, PF, SAMP, compress, decompress,
                       decompress_header, transform)

BASEPATH = Path("tests-data")

invariant_transforms = [
    [OP.ROT90, OP.ROT90, OP.ROT90, OP.ROT90],
    [OP.ROT180, OP.ROT180],
    [OP.ROT90, OP.ROT270],
    [OP.HFLIP, OP.HFLIP],
    [OP.VFLIP, OP.VFLIP],
    [OP.TRANSPOSE, OP.TRANSPOSE],
    [OP.TRANSVERSE, OP.TRANSVERSE],
    [OP.TRANSPOSE, OP.ROT90, OP.HFLIP],
    [OP.TRANSVERSE, OP.ROT90, OP.VFLIP],
    [OP.TRANSVERSE, OP.TRANSPOSE, OP.ROT180],
]


def do_transforms(data: bytes, transforms: List[OP]) -> bytes:
    for op in transforms:
        data = transform(data, op=op)
    return data


class TurbojpegTest(unittest.TestCase):
    def test_transform_invariances(self):
        paths = sorted(BASEPATH.glob("*.jpg"))
        self.assertGreater(len(paths), 0)
        for path in paths:
            truth = path.read_bytes()
            for transforms in invariant_transforms:
                with self.subTest(path=path, transforms=transforms):
                    result = do_transforms(truth, transforms)
                    self.assertEqual(truth, result)

    def test_compress(self):
        paths = sorted(BASEPATH.glob("*.png"))
        for path in paths:
            pil_png = np.array(Image.open(path))
            pil_jpg = np.array(Image.open(path.with_suffix(".jpg")))

            if pil_png.ndim == 2:
                result = compress(pil_png, 90, SAMP.GRAY)
            elif pil_png.ndim == 3:
                result = compress(pil_png, 90, SAMP.Y420)
            else:
                assert False

            arr_result = np.array(Image.open(BytesIO(result)))

            if arr_result.ndim == 2:
                channel_axis = None
            else:
                channel_axis = 2

            ssim = structural_similarity(pil_jpg, arr_result, channel_axis=channel_axis)
            self.assertAlmostEqual(ssim, 1.0)

    def test_decompress_fail(self):
        with self.assertRaisesRegex(
            RuntimeError, r"tj3DecompressHeader\(\): Invalid argument"
        ):
            decompress(b"")

        with self.assertRaisesRegex(RuntimeError, r"Unsupported buffer format: d"):
            decompress(np.array([], dtype=np.float64))

    def test_decompress_warnings(self):
        data = (BASEPATH / "warnings/gray_gradient_mod1.jpg").read_bytes()
        result = decompress(data)

        with self.assertRaisesRegex(
            RuntimeError, r"Corrupt JPEG data: 1 extraneous bytes before marker 0xd9"
        ):
            result = decompress(data, raise_on_warnings=True)

    def test_decompress_ssim(self):
        paths = sorted(BASEPATH.glob("*.jpg"))
        for path in paths:
            pil_jpg = np.array(Image.open(path))
            result = decompress(path.read_bytes())
            arr_result = np.array(result, copy=False)

            self.assertEqual(result.xdensity, 1)
            self.assertEqual(result.ydensity, 1)
            self.assertEqual(result.densityunits, DU.UNKNOWN)

            if arr_result.ndim == 2:
                self.assertEqual(result.colorspace, CS.GRAY)
                channel_axis = None
            else:
                self.assertEqual(result.colorspace, CS.YCbCr)
                channel_axis = 2

            ssim = structural_similarity(pil_jpg, arr_result, channel_axis=channel_axis)
            self.assertAlmostEqual(ssim, 1.0)

    def test_decompress_memory(self):
        paths = sorted(BASEPATH.glob("*.jpg"))
        for i in range(10):  # fixme: check mem growth
            for path in paths:
                bytes_result = decompress(path.read_bytes())
                arr_result = np.array(bytes_result, copy=False)

    def test_decompress_header(self):
        paths = sorted(BASEPATH.glob("*.jpg"))
        truths = [
            {
                "width": 256,
                "height": 256,
                "subsamp": 2,
                "colorspace": CS.YCbCr,
                "precision": 8,
                "progressive": 0,
                "lossless": 0,
                "xdensity": 1,
                "ydensity": 1,
                "densityunits": DU.UNKNOWN,
            },
            {
                "width": 256,
                "height": 256,
                "subsamp": 2,
                "colorspace": CS.YCbCr,
                "precision": 8,
                "progressive": 0,
                "lossless": 0,
                "xdensity": 1,
                "ydensity": 1,
                "densityunits": DU.UNKNOWN,
            },
            {
                "width": 256,
                "height": 256,
                "subsamp": 3,
                "colorspace": CS.GRAY,
                "precision": 8,
                "progressive": 0,
                "lossless": 0,
                "xdensity": 1,
                "ydensity": 1,
                "densityunits": DU.UNKNOWN,
            },
            {
                "width": 256,
                "height": 256,
                "subsamp": 3,
                "colorspace": CS.GRAY,
                "precision": 8,
                "progressive": 0,
                "lossless": 0,
                "xdensity": 1,
                "ydensity": 1,
                "densityunits": DU.UNKNOWN,
            },
        ]
        self.assertEqual(len(truths), len(paths))
        for truth, path in zip(truths, paths):
            result = decompress_header(path.read_bytes())
            self.assertEqual(truth, result)


if __name__ == "__main__":
    unittest.main()
