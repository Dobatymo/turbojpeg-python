import unittest
from pathlib import Path
from typing import List

import turbojpeg
from turbojpeg import OP, transform

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
    def test_invariances(self):
        paths = sorted(Path(".").glob("*.jpg"))
        self.assertGreater(len(paths), 0)
        for path in paths:
            truth = path.read_bytes()
            for transforms in invariant_transforms:
                with self.subTest(path=path, transforms=transforms):
                    result = do_transforms(truth, transforms)
                    self.assertEqual(truth, result)


if __name__ == "__main__":
    unittest.main()
