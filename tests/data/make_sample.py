import h5py
import numpy as np


enum_map = {
    "A": 1,
    "B": 2,
    "C": 3,
}

enum_dtype = h5py.enum_dtype(enum_map, basetype=np.int32)


with h5py.File("sample.h5", "w") as store:
    #
    # Scalar dataset
    #
    scalar_group = store.create_group("scalar")

    scalar_group["float"] = 3.14
    scalar_group["int"] = 1234
    scalar_group["string"] = "This is a string"

    scalar_group.create_dataset("enum", data=enum_map["A"], dtype=enum_dtype)

    #
    # Simple array dataset
    #
    simple_group = store.create_group("simple")

    data = np.arange(10) / 9
    simple_group.create_dataset("float_1", data=data, dtype=np.float32)

    row = np.arange(10) / 9
    col = np.arange(5) / 4
    data = col[:, None] - row[None, :]
    simple_group.create_dataset("float_2", data=data, dtype=np.float32)

    data = np.arange(10)
    simple_group.create_dataset("int_1", data=data, dtype=np.int32)

    row = np.arange(5)
    col = np.arange(10)
    data = col[:, None] - row[None, :]
    simple_group.create_dataset("int_2", data=data, dtype=np.int32)

    data = np.array([1, 2, 3, 2, 1])
    simple_group.create_dataset("enum", data=data, dtype=enum_dtype)
