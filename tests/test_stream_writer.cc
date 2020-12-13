
#include <algorithm>
#include <iterator>
#include <vector>

#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"


TEST_CASE("dataset::stream_writer - creates new dataset")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    // Open a non-existing path.
    h5::dataset<float, 3> dataset = file.dataset<float, 3>("data/foo/bar");
    CHECK_FALSE(dataset);

    // Opening a stream_writer creates a dataset.
    h5::shape<2> const record_shape = {2, 3};
    dataset.stream_writer(record_shape);
    CHECK(dataset);

    // The created dataset is empty.
    h5::shape<3> const expected_shape = {0, record_shape.dims[0], record_shape.dims[1]};
    CHECK(dataset.shape() == expected_shape);
}

TEST_CASE("stream_writer - incrementally writes arrays to disk")
{
    temporary tmp;
    h5::file file(tmp.filename, "w");

    // Open a non-existing path.
    h5::dataset<float, 3> dataset = file.dataset<float, 3>("data/foo/bar");
    CHECK_FALSE(dataset);

    // Write sequence of 2-dimensional arrays (records) incrementally.
    std::size_t const record_count = 10;
    h5::shape<2> const record_shape = {2, 3};
    std::vector<double> expected_data;
    {
        h5::stream_writer<h5::f32, 2> stream = dataset.stream_writer(record_shape);
        std::vector<double> record(record_shape.size());

        for (std::size_t i = 0; i < record_count; i++) {
            stream.write(record.data());
            std::copy(record.begin(), record.end(), std::back_inserter(expected_data));
        }
        stream.flush();
    }

    // Now we have expected number of records.
    h5::shape<3> const expected_shape = {
        record_count, record_shape.dims[0], record_shape.dims[1]
    };
    CHECK(dataset.shape() == expected_shape);

    // Check written data.
    std::vector<double> actual_data(expected_shape.size());
    dataset.read(actual_data.data(), expected_shape);
    CHECK(actual_data == expected_data);
}
