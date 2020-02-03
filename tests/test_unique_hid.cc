#include <type_traits>

#include <h5.hpp>

#include <catch.hpp>


TEST_CASE("unique_hid - is instantiable with HDF5 resource close funcs")
{
    h5::unique_hid<H5Aclose> attribute;
    h5::unique_hid<H5Dclose> dataset;
    h5::unique_hid<H5Fclose> file;
    h5::unique_hid<H5Gclose> group;
    h5::unique_hid<H5Pclose> props;
    h5::unique_hid<H5Sclose> dataspace;
    h5::unique_hid<H5Tclose> datatype;

    CHECK(attribute < 0);
    CHECK(dataset < 0);
    CHECK(file < 0);
    CHECK(group < 0);
    CHECK(props < 0);
    CHECK(dataspace < 0);
    CHECK(datatype < 0);
}

TEST_CASE("unique_hid - is implicitly convertible to hid value")
{
    hid_t const orig_hid = H5Screate(H5S_SCALAR);
    h5::unique_hid<H5Sclose> const hid = orig_hid;
    hid_t const decay_hid = hid;
    CHECK(decay_hid == orig_hid);
}

TEST_CASE("unique_hid - is non-copyable")
{
    using HID = h5::unique_hid<H5Sclose>;

    CHECK_FALSE(std::is_copy_constructible<HID>::value);
    CHECK_FALSE(std::is_copy_assignable<HID>::value);
}

TEST_CASE("unique_hid - is movable")
{
    using HID = h5::unique_hid<H5Sclose>;

    hid_t const orig_hid1 = H5Screate(H5S_SCALAR);
    hid_t const orig_hid2 = H5Screate(H5S_SCALAR);
    REQUIRE(orig_hid1 >= 0);
    REQUIRE(orig_hid2 >= 0);

    HID hid1 = orig_hid1;
    HID hid2 = orig_hid2;

    // Move construction
    HID hid3 = std::move(hid1);
    CHECK(hid1 < 0);
    CHECK(hid3 == orig_hid1);

    // Move assignment
    hid2 = std::move(hid3);
    CHECK(hid2 == orig_hid1);
    CHECK(hid3 < 0);
}

TEST_CASE("unique_hid::swap - swaps contained hid values")
{
    using HID = h5::unique_hid<H5Sclose>;

    hid_t const orig_hid1 = H5Screate(H5S_SCALAR);
    hid_t const orig_hid2 = H5Screate(H5S_SCALAR);
    REQUIRE(orig_hid1 >= 0);
    REQUIRE(orig_hid2 >= 0);

    HID hid1 = orig_hid1;
    HID hid2 = orig_hid2;

    hid1.swap(hid2);
    CHECK(hid1 == orig_hid2);
    CHECK(hid2 == orig_hid1);
}
