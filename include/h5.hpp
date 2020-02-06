// Copyright snsinfu 2020.
// Distributed under the Boost Software License, Version 1.0.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef INCLUDED_SNSINFU_H5_HPP
#define INCLUDED_SNSINFU_H5_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <hdf5.h>


namespace h5
{
    // EXCEPTION -------------------------------------------------------------

    // The exception class used to report an error.
    class exception : public std::runtime_error
    {
    public:
        explicit exception(std::string const& msg)
            : std::runtime_error{msg}
        {
        }
    };


    // UTILITY ---------------------------------------------------------------

    namespace detail
    {
        // Basic optional wrapper.
        template<typename T>
        class optional
        {
        public:
            optional() = default;

            optional(T const& value)
                : _value{std::make_unique<T>(value)}
            {
            }

            optional(optional const& other)
                : _value{other ? std::make_unique<T>(*other) : nullptr}
            {
            }

            optional& operator=(optional const& other)
            {
                optional tmp = other;
                _value.swap(tmp._value);
                return *this;
            }

            ~optional() = default;


            explicit operator bool() const noexcept
            {
                return bool(_value);
            }

            T& operator*() const
            {
                return *_value;
            }

        private:
            std::unique_ptr<T> _value;
        };
    }


    // RAII ------------------------------------------------------------------

    // Thin RAII wrapper for an hid_t.
    template<herr_t(& close_fn)(hid_t)>
    class unique_hid
    {
    public:
        // The default constructor creates an empty object.
        unique_hid() = default;

        // The constructor takes
        unique_hid(hid_t hid)
            : _hid{hid}
        {
        }

        ~unique_hid() noexcept
        {
            if (_hid >= 0) {
                close_fn(_hid);
            }
        }

        unique_hid(unique_hid const&) = delete;

        unique_hid(unique_hid&& other) noexcept
        {
            swap(other);
        }

        unique_hid& operator=(unique_hid&& other) noexcept
        {
            unique_hid tmp = std::move(other);
            swap(tmp);
            return *this;
        }

        void swap(unique_hid& other) noexcept
        {
            auto tmp = _hid;
            _hid = other._hid;
            other._hid = tmp;
        }

        operator hid_t() const noexcept
        {
            return _hid;
        }

    private:
        hid_t _hid = -1;
    };


    // DATA TYPES ------------------------------------------------------------

    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;

    namespace detail
    {
        // Returns variable-length UTF-8 string datatype.
        inline hid_t string_datatype()
        {
            static h5::unique_hid<H5Tclose> const string_type = [] {
                h5::unique_hid<H5Tclose> type = H5Tcopy(H5T_C_S1);
                if (type < 0) {
                    throw h5::exception("failed to copy H5T_C_S1");
                }
                if (H5Tset_cset(type, H5T_CSET_UTF8) < 0) {
                    throw h5::exception("failed to set UTF-8 charset");
                }
                if (H5Tset_size(type, H5T_VARIABLE) < 0) {
                    throw h5::exception("failed to set variable length");
                }
                return type;
            }();
            return string_type;
        }
    }


    // We want to map C++ scalar type to the corresponding HDF5 datatype. We
    // use function templates because datatype values are determined at run
    // time. (H5T_* macros are not constants!)

    template<typename T>
    hid_t storage_type() = delete;

    template<> inline hid_t storage_type<h5::i32>() { return H5T_STD_I32LE; }
    template<> inline hid_t storage_type<h5::i64>() { return H5T_STD_I64LE; }
    template<> inline hid_t storage_type<h5::u32>() { return H5T_STD_U32LE; }
    template<> inline hid_t storage_type<h5::u64>() { return H5T_STD_U64LE; }
    template<> inline hid_t storage_type<float>() { return H5T_IEEE_F32LE; }
    template<> inline hid_t storage_type<double>() { return H5T_IEEE_F64LE; }
    template<> inline hid_t storage_type<char*>() { return detail::string_datatype(); }

    template<typename T>
    hid_t memory_type() = delete;

    template<> inline hid_t memory_type<h5::i32>() { return H5T_NATIVE_INT32; }
    template<> inline hid_t memory_type<h5::i64>() { return H5T_NATIVE_INT64; }
    template<> inline hid_t memory_type<h5::u32>() { return H5T_NATIVE_UINT32; }
    template<> inline hid_t memory_type<h5::u64>() { return H5T_NATIVE_UINT64; }
    template<> inline hid_t memory_type<float>() { return H5T_NATIVE_FLOAT; }
    template<> inline hid_t memory_type<double>() { return H5T_NATIVE_DOUBLE; }
    template<> inline hid_t memory_type<char*>() { return detail::string_datatype(); }
    template<> inline hid_t memory_type<char const*>() { return detail::string_datatype(); }


    // SHAPE -----------------------------------------------------------------

    // Shape of a simple dataset (multi-dimensional array). The `rank` must be
    // positive (i.e., cannot be zero).
    template<int rank>
    struct shape
    {
        std::size_t dims[rank] = {};

        // Returns the total number of elements in the hypercube of this shape.
        std::size_t size() const noexcept
        {
            std::size_t prod = dims[0];
            for (int i = 1; i < rank; i++) {
                prod *= dims[i];
            }
            return prod;
        }
    };

    template<int rank>
    bool operator==(h5::shape<rank> const& s1, h5::shape<rank> const& s2)
    {
        for (int i = 0; i < rank; i++) {
            if (s1.dims[i] != s2.dims[i]) {
                return false;
            }
        }
        return true;
    }

    template<int rank>
    bool operator!=(h5::shape<rank> const& s1, h5::shape<rank> const& s2)
    {
        return !(s1 == s2);
    }


    // PATH ------------------------------------------------------------------

    namespace detail
    {
        // Returns the parent of `path`. Returns empty string if `path` has no
        // parent component.
        inline std::string parent_path(std::string const& path)
        {
            auto const sep_pos = path.rfind('/');
            if (sep_pos == std::string::npos) {
                return "";
            }
            return path.substr(0, sep_pos);
        }


        // Returns true if `path` exists in `file`.
        inline bool check_path_exists(hid_t file, std::string const& path)
        {
            auto const parent = detail::parent_path(path);
            if (!parent.empty()) {
                if (!check_path_exists(file, parent)) {
                    return false;
                }
            }

            auto const status = H5Lexists(file, path.c_str(), H5P_DEFAULT);
            if (status < 0) {
                throw h5::exception("failed to check if a path exists");
            }
            return status > 0;
        }
    }


    // AUTO CHUNKING ---------------------------------------------------------

    namespace detail
    {
        template<int rank>
        h5::shape<rank>
        determine_chunk_size(h5::shape<rank> const& shape, std::size_t value_size)
        {
            // Heuristic from h5py/PyTables.

            constexpr std::size_t KiB = 1024;
            constexpr std::size_t MiB = 1024 * 1024;
            constexpr std::size_t min_size = 8 * KiB;
            constexpr std::size_t base_size = 24 * KiB;
            constexpr std::size_t max_size = 1 * MiB;

            auto const data_size = shape.size() * value_size;
            auto const magnitude = std::log10(double(data_size) / MiB);
            auto const raw_threshold = base_size << int(magnitude);
            auto const threshold = std::min(std::max(raw_threshold, min_size), max_size);

            auto chunk = shape;

            for (int axis = 0; ; ++axis %= rank) {
                auto const chunk_size = chunk.size() * value_size;
                if (chunk_size < threshold) {
                    break;
                }

                chunk.dims[axis] = (chunk.dims[axis] + 1) / 2;
            }

            assert(chunk.size() > 0);

            return chunk;
        }
    }


    // DATASET HANDLING ------------------------------------------------------

    struct dataset_options
    {
        detail::optional<int> compression;
        detail::optional<int> scaleoffset;
    };


    namespace detail
    {
        // Checks the rank of a dataset. Throws an exception if the actual rank
        // is not the expected `rank`. Otherwise, returns the shape.
        template<int rank>
        h5::shape<rank> check_dataset_rank(hid_t dataset)
        {
            h5::unique_hid<H5Sclose> dataspace = H5Dget_space(dataset);
            if (dataspace < 0) {
                throw h5::exception("failed to determine dataspace");
            }

            auto const dataset_rank = H5Sget_simple_extent_ndims(dataspace);
            if (dataset_rank != rank) {
                throw h5::exception("unexpected dataset rank");
            }

            hsize_t dims[rank];
            if (H5Sget_simple_extent_dims(dataspace, dims, nullptr) < 0) {
                throw h5::exception("failed to determine dataset shape");
            }

            h5::shape<rank> shape;
            for (int i = 0; i < rank; i++) {
                shape.dims[i] = static_cast<std::size_t>(dims[i]);
            }
            return shape;
        }


        // Checks if the datatype of `dataset` is compatible with `D`. Throws
        // an exception if the types are incompatible. Returns a datatype hid
        // of the dataset on success.
        template<typename D>
        h5::unique_hid<H5Tclose> check_dataset_type(hid_t dataset)
        {
            h5::unique_hid<H5Tclose> datatype = H5Dget_type(dataset);
            if (datatype < 0) {
                throw h5::exception("failed to determine datatype");
            }

            H5T_cdata_t* cdata = nullptr;
            if (H5Tfind(datatype, h5::storage_type<D>(), &cdata) == nullptr) {
                throw h5::exception("incompatible dataset type");
            }

            return datatype;
        }


        template<typename D>
        H5Z_SO_scale_type_t determine_scaleoffset_type()
        {
            if (std::is_floating_point<D>::value) {
                return H5Z_SO_FLOAT_DSCALE;
            }
            if (std::is_integral<D>::value) {
                return H5Z_SO_INT;
            }
            throw h5::exception("cannot apply scaleoffset to specified datatype");
        }


        // Creates a new simple dataset.
        template<typename D, int rank>
        h5::unique_hid<H5Dclose> create_simple_dataset(
            hid_t file,
            std::string const& path,
            h5::shape<rank> const& shape,
            h5::dataset_options const& options
        )
        {
            hsize_t dims[rank];
            for (int i = 0; i < rank; i++) {
                dims[i] = static_cast<hsize_t>(shape.dims[i]);
            }

            h5::unique_hid<H5Sclose> dataspace = H5Screate_simple(rank, dims, nullptr);
            if (dataspace < 0) {
                throw h5::exception("failed to create dataspace");
            }

            // Allow intermediate groups to be automatically created.
            h5::unique_hid<H5Pclose> link_props = H5Pcreate(H5P_LINK_CREATE);
            if (link_props < 0) {
                throw h5::exception("failed to create link props");
            }
            if (H5Pset_create_intermediate_group(link_props, 1) < 0) {
                throw h5::exception("failed to configure link props");
            }

            // Optional filters.
            h5::unique_hid<H5Pclose> dataset_props = H5Pcreate(H5P_DATASET_CREATE);
            if (dataset_props < 0) {
                throw h5::exception("failed to create dataset props");
            }

            if (options.compression || options.scaleoffset) {
                auto const chunk = detail::determine_chunk_size(shape, sizeof(D));

                hsize_t chunk_dims[rank];
                for (int i = 0; i < rank; i++) {
                    chunk_dims[i] = chunk.dims[i];
                }
                if (H5Pset_chunk(dataset_props, rank, chunk_dims) < 0) {
                    throw h5::exception("failed to set chunk size");
                }
            }

            if (options.scaleoffset) {
                auto const type = detail::determine_scaleoffset_type<D>();
                auto const factor = *options.scaleoffset;

                if (H5Pset_scaleoffset(dataset_props, type, factor) < 0) {
                    throw h5::exception("failed to set shuffle filter");
                }
            }

            if (options.compression) {
                auto const level = static_cast<unsigned>(*options.compression);

                if (H5Pset_shuffle(dataset_props) < 0) {
                    throw h5::exception("failed to set shuffle filter");
                }
                if (H5Pset_deflate(dataset_props, level) < 0) {
                    throw h5::exception("failed to set deflate filter");
                }
            }

            h5::unique_hid<H5Dclose> dataset = H5Dcreate2(
                file,
                path.c_str(),
                h5::storage_type<D>(),
                dataspace,
                link_props,
                dataset_props,
                H5P_DEFAULT
            );
            if (dataset < 0) {
                throw h5::exception("failed to create dataset");
            }

            return dataset;
        }
    }


    // Provides read/write access to an HDF5 dataset.
    //
    // The type `D` asserts the expected datatype on disk. `rank` asserts the
    // expected rank of the dataset. Zero `rank` means a scalar dataset.
    //
    template<typename D, int rank>
    class dataset
    {
    public:
        static_assert(rank > 0, "rank must be positive");


        // Tries to open a simple dataset on the `path` in `file`.
        //
        // If the path does not exist, the constructor just initializes the
        // object in the empty state. The object can be used to create a new
        // dataset on the path by calling `write`.
        //
        // If the path is not a simple dataset, or if the type and/or rank
        // assertion fails, the constructor throws an `h5::exception`.
        //
        // The behavior is undefined if a `dataset` object outlives `file`.
        // Make sure `dataset` is destroyed before the file it originates.
        //
        dataset(hid_t file, std::string const& path)
            : _file{file}, _path{path}
        {
            if (detail::check_path_exists(file, path)) {
                _dataset = H5Dopen2(file, path.c_str(), H5P_DEFAULT);
                if (_dataset < 0) {
                    throw h5::exception("failed to open dataset");
                }

                detail::check_dataset_rank<rank>(_dataset);
                detail::check_dataset_type<D>(_dataset);
            }
        }


        // Returns `true` if the object holds a dataset.
        explicit operator bool() const noexcept
        {
            return _dataset >= 0;
        }


        // Returns the underlying dataset HID. Returns -1 if the object does
        // not hold a dataset.
        hid_t handle() const noexcept
        {
            return _dataset;
        }


        // Retrieves the shape of the dataset. Returns a zero shape if the
        // object does not hold a dataset.
        h5::shape<rank> shape() const noexcept
        {
            if (_dataset < 0) {
                return {};
            }
            return detail::check_dataset_rank<rank>(_dataset);
        }


        // Reads all data from the dataset.
        //
        template<typename T>
        void read(T* buf, h5::shape<rank> const& shape)
        {
            if (this->shape() != shape) {
                throw h5::exception("shape mismatch when reading");
            }

            auto const status = H5Dread(
                _dataset, h5::memory_type<T>(), H5S_ALL, H5S_ALL, H5P_DEFAULT, buf
            );
            if (status < 0) {
                throw h5::exception("failed to read from dataset");
            }
        }


        // Writes a new dataset of given shape.
        //
        // The function writes flattened data pointed-to by `buf` to the path.
        // It always creates a new dataset, clobbering existing one if any.
        // Ancestor groups are created if not exist.
        //
        // XXX: Current implementation is not exception safe. Old dataset will
        // be lost if writing a new dataset fails.
        //
        template<typename T>
        void write(
            T const* buf,
            h5::shape<rank> const& shape,
            h5::dataset_options const& options
        )
        {
            if (detail::check_path_exists(_file, _path)) {
                if (H5Ldelete(_file, _path.c_str(), H5P_DEFAULT) < 0) {
                    throw h5::exception("failed to delete a path");
                }
            }
            _dataset = -1;
            _dataset = detail::create_simple_dataset<D, rank>(
                _file, _path, shape, options
            );

            auto const status = H5Dwrite(
                _dataset,
                h5::memory_type<T>(),
                H5S_ALL,
                H5S_ALL,
                H5P_DEFAULT,
                buf
            );
            if (status < 0) {
                throw h5::exception("failed to write to dataset");
            }

            if (H5Fflush(_file, H5F_SCOPE_LOCAL) < 0) {
                throw h5::exception("failed to flush changes to disk");
            }
        }


        // Calls `write` with default options.
        template<typename T>
        void write(T const* buf, h5::shape<rank> const& shape)
        {
            h5::dataset_options default_options;
            return write(buf, shape, default_options);
        }


    private:
        hid_t _file;
        std::string _path;
        h5::unique_hid<H5Dclose> _dataset;
    };


    // FILE HANDLING ---------------------------------------------------------

    namespace detail
    {
        // Opens an existing HDF5 file.
        inline
        h5::unique_hid<H5Fclose>
        do_open_file(std::string const& filename, bool readonly)
        {
            h5::unique_hid<H5Fclose> file = H5Fopen(
                filename.c_str(),
                readonly ? H5F_ACC_RDONLY : H5F_ACC_RDWR,
                H5P_DEFAULT
            );
            if (file < 0) {
                throw h5::exception("cannot open file");
            }
            return file;
        }


        // Creates an empty HDF5 file.
        inline
        h5::unique_hid<H5Fclose>
        do_create_file(std::string const& filename, bool truncate)
        {
            h5::unique_hid<H5Fclose> file = H5Fcreate(
                filename.c_str(),
                truncate ? H5F_ACC_TRUNC : H5F_ACC_EXCL,
                H5P_DEFAULT,
                H5P_DEFAULT
            );
            if (file < 0) {
                throw h5::exception("cannot create file");
            }
            return file;
        }


        // Opens or creates an HDF5 file based on given mode string.
        inline
        h5::unique_hid<H5Fclose>
        open_file(std::string const& filename, std::string const& mode)
        {
            if (mode == "r") {
                return detail::do_open_file(filename, true);
            }
            if (mode == "r+") {
                return detail::do_open_file(filename, false);
            }
            if (mode == "w") {
                return detail::do_create_file(filename, true);
            }
            if (mode == "w-") {
                return detail::do_create_file(filename, false);
            }
            throw h5::exception("unrecognized file mode");
        }
    }


    // A `file` object provides read/write access to datasets in an HDF5 file.
    class file
    {
    public:
        // Opens or creates an HDF5 file.
        //
        // Parameters:
        //   filename = Path to the HDF5 file.
        //   mode     = One of these four strings: r, r+, w or w-.
        //
        // | Mode | Meaning                                   |
        // |------|-------------------------------------------|
        // | r    | Read only. File must exist.               |
        // | r+   | Read-write. File must exist.              |
        // | w    | Read-write. File is created or truncated. |
        // | w-   | Read-write. File must not exist.          |
        //
        file(std::string const& filename, std::string const& mode)
            : _file{detail::open_file(filename, mode)}
        {
        }


        // Returns the underlying file HID.
        hid_t handle() const noexcept
        {
            return _file;
        }


        // Opens `path` on the file for reading or writing a dataset.
        //
        // Parameters:
        //   D    = Expected type of the dataset elements.
        //   rank = Expected rank of the dataset (0: scalar, 1: vector,
        //          2: matrix, ...).
        //   path = HDF5 dataset path.
        //
        // Returns:
        //   `h5::dataset` object.
        //
        template<typename D, int rank = 0>
        h5::dataset<D, rank> dataset(std::string const& path)
        {
            return h5::dataset<D, rank>{_file, path};
        }

    private:
        h5::unique_hid<H5Fclose> _file;
    };
}

#endif
