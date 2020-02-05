#include <algorithm>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"


TEST_CASE("dataset - opens existing dataset")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    CHECK(file.dataset<int, 1>("simple/int_1").handle() >= 0);
    CHECK(file.dataset<int, 2>("simple/int_2").handle() >= 0);
    CHECK(file.dataset<float, 1>("simple/float_1").handle() >= 0);
    CHECK(file.dataset<float, 2>("simple/float_2").handle() >= 0);
}

TEST_CASE("dataset::write - creates new dataset")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    // Open a non-existing path.
    h5::dataset<float, 3> dataset = file.dataset<float, 3>("data/foo/bar");
    CHECK(dataset.handle() < 0);

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

    // Now the dataset should have been resized.
    CHECK(dataset.shape() == shape);

    // TODO: read
}

TEST_CASE("dataset::write - replaces existing dataset")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r+");

    // Open an existing dataset.
    h5::dataset<float, 2> dataset = file.dataset<float, 2>("simple/float_2");
    CHECK(dataset.handle() >= 0);

    // Prepare data.
    h5::shape<2> const shape = {10, 2};

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
    CHECK(dataset.shape() == shape);
}
