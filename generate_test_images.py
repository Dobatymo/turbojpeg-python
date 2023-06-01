import numpy as np
from PIL import Image


def gray_gradient():
    arr = np.broadcast_to(np.arange(0, 256, dtype=np.uint8).reshape(1, 256), (256, 256))
    return arr


def color_gradient():
    arr = np.empty((256, 256, 3), dtype=np.uint8)
    arr[..., 0] = np.broadcast_to(
        np.arange(0, 256, dtype=np.uint8).reshape(1, 256), (256, 256)
    ).T
    arr[..., 1] = np.broadcast_to(
        np.arange(0, 256, dtype=np.uint8).reshape(1, 256), (256, 256)
    )
    arr[..., 2] = np.broadcast_to(
        np.arange(255, -1, -1, dtype=np.uint8).reshape(1, 256), (256, 256)
    )
    return arr


def gray_random():
    arr = np.random.randint(0, 256, (256, 256), dtype=np.uint8)
    return arr


def color_random():
    arr = np.random.randint(0, 256, (256, 256, 3), dtype=np.uint8)
    return arr


if __name__ == "__main__":
    pics = [gray_random, gray_gradient, color_random, color_gradient]

    for func in pics:
        arr = func()
        img = Image.fromarray(arr)
        img.save(f"tests/{func.__name__}.jpg", quality=90)
