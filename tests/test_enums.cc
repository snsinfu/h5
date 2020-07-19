#include <vector>

#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"



TEST_CASE("enums - is default constructible")
{
    h5::enums<int> enums;
    CHECK(enums.size() == 0);
}

TEST_CASE("enums - accepts initializer list")
{
    h5::enums<int> enums = {
        {"A", 1}, {"B", 2}, {"C", 3}, {"D", 4}
    };
    CHECK(enums.size() == 4);

    CHECK(!enums.value("nonexistent"));
    CHECK(*enums.value("A") == 1);
    CHECK(*enums.value("B") == 2);
    CHECK(*enums.value("C") == 3);
    CHECK(*enums.value("D") == 4);

    CHECK(!enums.name(0));
    CHECK(*enums.name(1) == "A");
    CHECK(*enums.name(2) == "B");
    CHECK(*enums.name(3) == "C");
    CHECK(*enums.name(4) == "D");
}

TEST_CASE("enums::insert - inserts a member")
{
    h5::enums<int> enums;

    enums.insert("A", 1);
    enums.insert("C", 3);
    enums.insert("B", 2);

    CHECK(enums.size() == 3);
    CHECK(*enums.value("A") == 1);
    CHECK(*enums.value("B") == 2);
    CHECK(*enums.value("C") == 3);
}

TEST_CASE("dataset - validates enum datatype")
{
    h5::file file("data/sample.h5", "r");

    // Correct enum
    h5::enums<h5::i32> const enums_truth = {
        {"A", 1}, {"B", 2}, {"C", 3}
    };

    h5::enums<h5::i32> const enums_wrong_key = {
        {"X", 1}, {"Y", 2}, {"Z", 3}
    };

    h5::enums<h5::i32> const enums_wrong_value = {
        {"A", 0}, {"B", 1}, {"C", 2}
    };

    h5::enums<h5::i32> const enums_missing_member = {
        {"A", 1}, {"C", 3}
    };

    h5::enums<h5::i8> const enums_wrong_type = {
        {"A", 1}, {"B", 2}, {"C", 3}
    };

    // Enum datatype is validated against expected members. Note: The negative
    // tests would produce HDF5 diagnosis messages to stderr.
    std::string const path = "simple/enum";

    CHECK_NOTHROW(file.dataset<h5::i32, 1>(path, enums_truth));
    CHECK_THROWS(file.dataset<h5::i32, 1>(path, enums_wrong_key));
    CHECK_THROWS(file.dataset<h5::i32, 1>(path, enums_wrong_value));
    CHECK_THROWS(file.dataset<h5::i32, 1>(path, enums_missing_member));
    CHECK_THROWS(file.dataset<h5::i8, 1>(path, enums_wrong_type));

    h5::dataset<h5::i32, 1> dataset = file.dataset<h5::i32, 1>(path, enums_truth);
    h5::shape<1> const shape = dataset.shape();

    std::vector<h5::i32> const expect = {
        1, 2, 3, 2, 1
    };

    REQUIRE(shape.dims[0] == expect.size());

    // Can read enum data as int array.
    std::vector<h5::i32> actual(shape.dims[0]);
    dataset.read(actual);

    CHECK(actual == expect);
}

TEST_CASE("dataset - creates enum dataset")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    h5::enums<h5::i32> const enums = {
        {"A", 1}, {"B", 2}, {"C", 3}
    };
    std::string const path = "data";

    std::vector<h5::i32> const expect = { 1, 2, 3, 2, 1 };
    std::vector<h5::i32> actual(expect.size());

    file.dataset<h5::i32, 1>("data", enums).write(expect);
    file.dataset<h5::i32, 1>("data", enums).read(actual);

    CHECK(actual == expect);
}

TEST_CASE("dataset::read - can convert enum value type")
{
    h5::file file("data/sample.h5", "r");

    // The sample enum dataset is based on i32. We load the dataset as an
    // array of i16 values. libhdf5 should handle conversion.
    h5::enums<h5::i32> const enums = {
        {"A", 1}, {"B", 2}, {"C", 3}
    };

    std::vector<h5::i16> const expect = { 1, 2, 3, 2, 1 };
    std::vector<h5::i16> actual;

    file.dataset<h5::i32, 1>("simple/enum", enums).read_fit(actual);
    CHECK(actual == expect);
}
