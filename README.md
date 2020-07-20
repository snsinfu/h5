# Simple C++ library for HDF5

[![Build Status][travis-badge]][travis-url]
[![Boost License][license-badge]][license-url]
![C++14,17,20][cxx-badge]

`h5` is a simple, header-only C++ library for reading/writing HDF5 datasets.

- [Install](#install)
- [Usage](#usage)
- [Testing](#testing)
- [License](#license)

[cxx-badge]: https://img.shields.io/badge/C%2B%2B-14%2F17%2F20-orange.svg
[license-badge]: https://img.shields.io/badge/license-Boost-blue.svg
[license-url]: https://github.com/snsinfu/h5/blob/master/LICENSE.txt
[travis-badge]: https://travis-ci.org/snsinfu/h5.svg?branch=master
[travis-url]: https://travis-ci.org/snsinfu/h5

## Install

Download the single header file [h5.hpp][raw] into your include directory:

```
curl -Lo include/h5.hpp https://github.com/snsinfu/h5/raw/master/include/h5.hpp
```

Alternatively, add this repository to your project as a git submodule and add
`-I submodules/h5/include` to compile option:

```
git submodule add https://github.com/snsinfu/h5 submodules/h5
```

[raw]: https://github.com/snsinfu/h5/raw/master/include/h5.hpp

## Usage

The usage is as easy as this:

```c++
h5::file file("data.h5", "r+");

// h5 operates on multi-dimensional array in C layout. Here's a buffer
// containing a 10x10x10 tensor.
std::vector<float> buf(1000);
h5::shape<3> shape = {10, 10, 10};

// Write the 10x10x10 dataset.
file.dataset<float, 3>("my/new/dataset").write(buf.data(), shape);

// Read back into the buffer.
file.dataset<float, 3>("my/new/dataset").read(buf.data(), shape);
```

API:

- [h5::file](#h5file)
  - [file::file(filename, mode)](#filefilefilename-mode)
  - [file::dataset<D, rank>(path, enums)](#filedatasetd-rankpath-enums)
- [h5::dataset](#h5dataset)
  - [dataset::shape()](#datasetshape)
  - [dataset::read(buf, shape)](#datasetreadbuf-shape)
  - [dataset::read(buf)](#datasetreadbuf)
  - [dataset::read_fit(buf)](#datasetread_fitbuf)
  - [dataset::write(buf, shape, options)](#datasetwritebuf-shape-options)
  - [dataset::write(buf, options)](#datasetwritebuf-options)
- [h5::enums](#h5enums)
  - [enums::enums(members)](#enumsenumsmembers)
  - [enums::insert(name, value)](#enumsinsertname-value)
- [h5::buffer_traits](#h5buffer_traits)

### h5::file

Represents an HDF5 file.

```c++
class h5::file {
    file(
        std::string const& filename,
        std::string const& mode
    );

    template<typename D, int rank>
    h5::dataset<D, rank> dataset(
        std::string const&  path,
        h5::enums<D> const& enums  // optional
    );
};
```

#### file::file(filename, mode)

Opens or creates an HDF5 file.

- `filename` - The filename of the HDF5 file to operate on.
- `mode` - Open mode: r, r+, w or w-.

| Mode | Description                          |
|------|--------------------------------------|
| r    | Read-only.                           |
| r+   | Read-write.                          |
| w    | Read-write. Truncates existing file. |
| w-   | Read-write. Fails when file exists.  |

#### file::dataset<D, rank>(path, enums)

Opens a dataset at `path` in the file.

The type `D` and integer `rank` are assertions on the accessed dataset. The
function throws an exception if the dataset has incompatible type with `D` or
the rank is not as specified. An optional argument `enums` specifies additional
assertion for enum-typed dataset (see [h5::enums](#h5enums)).

This function does not fail if `path` does not exist. In that case the returned
dataset object is in "empty" state, disallowing `read` and allowing `write`.

Expected dataset types:

| D       | Description                |
|---------|----------------------------|
| h5::i8  | 8-bit signed jinteger      |
| h5::i16 | 16-bit signed integer      |
| h5::i32 | 32-bit signed integer      |
| h5::i64 | 64-bit signed integer      |
| h5::u8  | 8-bit unsigned jinteger    |
| h5::u16 | 16-bit unsigned integer    |
| h5::u32 | 32-bit unsigned integer    |
| h5::u64 | 64-bit unsigned integer    |
| h5::f32 | 32-bit IEEE floating-point |
| h5::f64 | 64-bit IEEE floating-point |
| h5::str | C-style UTF-8 string       |

### h5::dataset

Represents an HDF5 dataset with known datatype and rank.

```c++
class h5::dataset<D, rank> {
    h5::shape<rank> shape() const;

    template<typename T>
    void read(
        T*                     buf,
        h5::shape<rank> const& shape
    );

    template<typename T>
    void write(
        T const*                   buf,
        h5::shape<rank> const&     shape,
        h5::dataset_options const& options  // optional
    );
};
```

#### dataset::shape()

Returns the shape of the dataset if exists.

#### dataset::read(buf, shape)

Reads dataset into a buffer. The dataset must exist.

- `buf` - Pointer to the beginning of a buffer.
- `shape` - The shape of the buffer. This must be the same as the shape of the
  dataset.

The buffer type `T` may be different from the dataset type `D` as long as the
conversion is supported by the HDF5 library.

One exception is `T = std::string` which this library supports conversion from
`D = h5::str` dataset (internally it is `char*`).

#### dataset::read(buf)

Reads dataset into a buffer. This function works the same way as
`dataset::read(buf, shape)` but uses [buffer traits](#h5buffer_traits) to
extract the pointer and the shape of the buffer object (a `std::vector` or a
user-defined one).

#### dataset::read_fit(buf)

Reads dataset into a buffer with auto-resizing. This functionion is similar
to `dataset::read(buf)` but this one automatically resizes `buf` so that the
dataset fits in the buffer. The buffer must support `buffer_traits::reshape`
trait.

#### dataset::write(buf, shape, options)

Writes data in a buffer to the dataset. This function always creates a new
dataset and overwrites existing one if any.

- `buf` - Pointer to the beginning of a buffer containing the data.
- `shape` - The shape of the buffer.
- `options` - Options (like compression) for the dataset created. This
  parameter is optional.

The buffer type `T` may be different from the dataset type `D` as long as the
conversion is supported by the HDF5 library.

One exception is `T = std::string` which this library supports conversion to
`D = h5::str` dataset (internally it is `char*`).

| Option      | Description                           |
|-------------|---------------------------------------|
| compression | Deflate compression level (0-9).      |
| scaleoffset | Scaleoffset lossy compression factor. |

#### dataset::write(buf, options)

Writes data in a buffer to the dataset. This function works the same way as
`dataset::write(buf, shape, options)` but uses [buffer traits](#h5buffer_traits)
to extract the pointer and the shape of the buffer object (a `std::vector`
or a user-defined one).

### h5::enums

Holds a list of enumerated, named integers. Used to define an enum datatype
when accessing an enum-valued dataset.

```c++
class h5::enums<D> {
    struct member {
        std::string name;
        D           value;
    };

    enums(
      std::initializer_list<member> members  // optional
    );

    void insert(
      std::string const& name,
      D                  value
    );
};
```

#### enums::enums(members)

Creates a list of enumerated values having given members. A member is a pair
of a string name and an integral value of type `D`. The list is empty if
`members` is not given.

#### enums::insert(name, value)

Inserts a member to the enum list.

### h5::buffer_traits

Customizable traits for defining buffer types used to read/write dataset. By
default one-dimensional buffer based on `std::vector` is defined.

```c++
struct h5::buffer_traits<B> {
  static constexpr int rank;

  using value_type = ...;

  static h5::shape<rank> shape(
    B const& buffer
  );

  static value_type const* data(
    B const& buffer
  );

  static value_type* data(
    B& buffer
  );

  // Optional
  static void reshape(
    B&                     buffer,
    h5::shape<rank> const& shape
  );
};
```

## Testing

Tests require POSIX environment and hdf5 development files.

```sh
git clone https://github.com/snsinfu/h5.git
cd h5/tests
make
```

## License

Boost Software License, Version 1.0.
