#include <algorithm>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"


namespace
{
    template<typename D, typename T>
    std::vector<T> roundtrip(h5::file& file, std::vector<T> const& data)
    {
        auto dataset = file.dataset<D, 1>("data");
        dataset.write(data.data(), {data.size()});

        std::vector<T> buf(dataset.shape().dims[0]);
        dataset.read(buf.data(), {buf.size()});

        return buf;
    }

    template<typename D, typename T>
    T roundtrip_scalar(h5::file& file, T const& data)
    {
        auto dataset = file.dataset<D>("data");
        dataset.write(data);

        T buf;
        dataset.read(buf);

        return buf;
    }
}


TEST_CASE("dataset - opens existing dataset")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    CHECK(file.dataset<int>("scalar/int"));
    CHECK(file.dataset<float>("scalar/float"));
    CHECK(file.dataset<int, 1>("simple/int_1"));
    CHECK(file.dataset<int, 2>("simple/int_2"));
    CHECK(file.dataset<float, 1>("simple/float_1"));
    CHECK(file.dataset<float, 2>("simple/float_2"));
}

TEST_CASE("dataset::read - reads existing dataset")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    SECTION("int scalar")
    {
        int const expect = 1234;
        h5::dataset<int, 0> dataset = file.dataset<int>("scalar/int");
        REQUIRE(dataset);

        int actual;
        dataset.read(actual);
        CHECK(actual == expect);
    }

    SECTION("int vector")
    {
        std::vector<int> const expect = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        };
        h5::dataset<int, 1> dataset = file.dataset<int, 1>("simple/int_1");
        REQUIRE(dataset);
        REQUIRE(dataset.shape() == h5::shape<1>{10});

        std::vector<int> actual(10);
        dataset.read(actual.data(), {10});
        CHECK(actual == expect);
    }

    SECTION("int matrix")
    {
        std::vector<int> const expect = {
            0, -1, -2, -3, -4,
            1, 0, -1, -2, -3,
            2, 1, 0, -1, -2,
            3, 2, 1, 0, -1,
            4, 3, 2, 1, 0,
            5, 4, 3, 2, 1,
            6, 5, 4, 3, 2,
            7, 6, 5, 4, 3,
            8, 7, 6, 5, 4,
            9, 8, 7, 6, 5
        };
        h5::dataset<int, 2> dataset = file.dataset<int, 2>("simple/int_2");
        REQUIRE(dataset);
        REQUIRE(dataset.shape() == h5::shape<2>{10, 5});

        std::vector<int> actual(50);
        dataset.read(actual.data(), h5::shape<2>{10, 5});
        CHECK(actual == expect);
    }
}

TEST_CASE("dataset::read - throws if dataset does not exist")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    h5::dataset<int, 2> dataset = file.dataset<int, 2>("non-existing-dataset");
    REQUIRE_FALSE(dataset);

    int buf[50];
    CHECK_THROWS_AS(dataset.read(buf, {5, 10}), h5::exception);
}

TEST_CASE("dataset::read - throws if shapes mismatch")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    h5::dataset<int, 2> dataset = file.dataset<int, 2>("simple/int_2");
    REQUIRE(dataset.shape() == h5::shape<2>{10, 5});

    int buf[50];
    CHECK_NOTHROW(dataset.read(buf, {10, 5}));
    CHECK_THROWS_AS(dataset.read(buf, {5, 10}), h5::exception);
}

TEST_CASE("dataset::write - creates new dataset")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    // Open a non-existing path.
    h5::dataset<float, 3> dataset = file.dataset<float, 3>("data/foo/bar");
    CHECK_FALSE(dataset);

    // Write data.
    h5::shape<3> const shape = {10, 2, 3};

    std::vector<double> data;
    std::mt19937 random;
    std::generate_n(
        std::back_inserter(data),
        shape.size(),
        [&] {
            std::normal_distribution<double> normal;
            return normal(random);
        }
    );

    dataset.write(data.data(), shape);

    // Now the dataset exists and has the expected size.
    CHECK(dataset);
    CHECK(dataset.shape() == shape);
}

TEST_CASE("dataset::write - replaces existing dataset")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r+");

    // Open an existing dataset.
    h5::dataset<float, 2> dataset = file.dataset<float, 2>("simple/float_2");
    CHECK(dataset);

    // Overwrite the dataset.
    h5::shape<2> const shape = {100, 100};

    std::vector<double> data;
    std::mt19937 random;
    std::generate_n(
        std::back_inserter(data),
        shape.size(),
        [&] {
            std::normal_distribution<double> normal;
            return normal(random);
        }
    );

    dataset.write(data.data(), shape);

    // Now the dataset should have been resized.
    CHECK(dataset);
    CHECK(dataset.shape() == shape);
}

TEST_CASE("dataset::write - applies compression")
{
    // Write large-ish data with and without compression.
    h5::shape<3> const shape = {10000, 2, 3};

    std::vector<double> data;
    std::mt19937 random;
    std::generate_n(
        std::back_inserter(data),
        shape.size(),
        [&] {
            std::normal_distribution<double> normal;
            return normal(random);
        }
    );

    SECTION("deflate")
    {
        temporary tmp_raw;
        temporary tmp_com;
        h5::file file_raw(tmp_raw.filename, "w");
        h5::file file_com(tmp_com.filename, "w");

        h5::dataset_options options;
        options.compression = 4;
        file_raw.dataset<float, 3>("data").write(data.data(), shape);
        file_com.dataset<float, 3>("data").write(data.data(), shape, options);

        // Compression reduces the size.
        hsize_t raw_size;
        hsize_t com_size;
        REQUIRE(H5Fget_filesize(file_raw.handle(), &raw_size) >= 0);
        REQUIRE(H5Fget_filesize(file_com.handle(), &com_size) >= 0);
        CHECK(raw_size > com_size);
    }

    SECTION("scale-offset")
    {
        temporary tmp_raw;
        temporary tmp_com;
        h5::file file_raw(tmp_raw.filename, "w");
        h5::file file_com(tmp_com.filename, "w");

        h5::dataset_options options;
        options.scaleoffset = 3;
        file_raw.dataset<float, 3>("data").write(data.data(), shape);
        file_com.dataset<float, 3>("data").write(data.data(), shape, options);

        // Compression reduces the size.
        hsize_t raw_size;
        hsize_t com_size;
        REQUIRE(H5Fget_filesize(file_raw.handle(), &raw_size) >= 0);
        REQUIRE(H5Fget_filesize(file_com.handle(), &com_size) >= 0);
        CHECK(raw_size > com_size);
    }

    SECTION("scale-offset + deflate")
    {
        temporary tmp_raw;
        temporary tmp_com;
        h5::file file_raw(tmp_raw.filename, "w");
        h5::file file_com(tmp_com.filename, "w");

        h5::dataset_options options;
        options.compression = 4;
        options.scaleoffset = 3;
        file_raw.dataset<float, 3>("data").write(data.data(), shape);
        file_com.dataset<float, 3>("data").write(data.data(), shape, options);

        // Compression reduces the size.
        hsize_t raw_size;
        hsize_t com_size;
        REQUIRE(H5Fget_filesize(file_raw.handle(), &raw_size) >= 0);
        REQUIRE(H5Fget_filesize(file_com.handle(), &com_size) >= 0);
        CHECK(raw_size > com_size);
    }
}

TEST_CASE("dataset - can read and write numeric and string scalar")
{
    SECTION("i32")
    {
        h5::i32 const expect = 12345678;
        h5::i32 actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip_scalar<h5::i32>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("u32")
    {
        h5::u32 const expect = 0x12345678;
        h5::u32 actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip_scalar<h5::u32>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("f32")
    {
        float const expect = 1.23456F;
        float actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip_scalar<float>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("f64")
    {
        double const expect = 1.23456789012345;
        double actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip_scalar<double>(file, expect);
        CHECK(actual == expect);
    }
}

TEST_CASE("dataset - can read and write numeric and string array")
{
    SECTION("i32")
    {
        std::vector<h5::i32> const expect = {
            12345678, -90123456, 78901234, -56789012, 34567890,
            -12345678, 90123456, -78901234, 56789012, -34567890
        };
        std::vector<h5::i32> actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip<h5::i32>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("u32")
    {
        std::vector<h5::u32> const expect = {
            0x12345678, 0x90123456, 0x78901234, 0x56789012, 0x34567890,
            0xabcdefab, 0xcdefabcd, 0xefabcdef
        };
        std::vector<h5::u32> actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip<h5::u32>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("f32")
    {
        std::vector<float> const expect = {
            1.23456F, -7.89012F, 3.45678F, -9.01234F, 5.67890F,
            -1.23456F, 7.89012F, -3.45678F, 9.01234F, -5.67890F
        };
        std::vector<float> actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip<float>(file, expect);
        CHECK(actual == expect);
    }

    SECTION("f64")
    {
        std::vector<double> const expect = {
            1.2345678901234, -5.6789012345678, 9.0123456789012,
            -3.4567890123456, 7.8901234567890, -1.2345678901234,
            5.6789012345678, -9.0123456789012, 3.4567890123456
        };
        std::vector<double> actual;

        temporary tmp;
        h5::file file(tmp.filename, "w");
        actual = roundtrip<double>(file, expect);
        CHECK(actual == expect);
    }
}

TEST_CASE("dataset - D parameter")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    file.dataset<h5::i8>("i8").write(0);
    file.dataset<h5::i16>("i16").write(0);
    file.dataset<h5::i32>("i32").write(0);
    file.dataset<h5::i64>("i64").write(0);
    file.dataset<h5::u8>("u8").write(0);
    file.dataset<h5::u16>("u16").write(0);
    file.dataset<h5::u32>("u32").write(0);
    file.dataset<h5::u64>("u64").write(0);
}
