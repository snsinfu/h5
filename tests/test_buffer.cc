#include <vector>

#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"


namespace
{
    struct triple_row
    {
        double x;
        double y;
        double z;
    };
    static_assert(sizeof(triple_row) == 3 * sizeof(double), "unexpected padding");
}

namespace h5
{
    template<>
    struct buffer_traits<std::vector<triple_row>>
    {
        using buffer_type = std::vector<triple_row>;
        using value_type = double;
        static constexpr int rank = 2;

        static h5::shape<rank> shape(buffer_type const& buffer)
        {
            return {buffer.size(), 3};
        }

        static void reshape(buffer_type& buffer, h5::shape<rank> const& shape)
        {
            buffer.resize(shape.dims[0]);
        }

        static value_type* data(buffer_type& buffer)
        {
            return reinterpret_cast<value_type*>(buffer.data());
        }

        static value_type const* data(buffer_type const& buffer)
        {
            return reinterpret_cast<value_type const*>(buffer.data());
        }
    };
}


TEST_CASE("dataset::read - accepts vector buffer")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "r");

    std::vector<int> const expect = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    h5::dataset<int, 1> dataset = file.dataset<int, 1>("simple/int_1");
    REQUIRE(dataset);
    REQUIRE(dataset.shape() == h5::shape<1>{10});

    std::vector<int> buffer;
    dataset.read(buffer);
    CHECK(buffer == expect);
}

TEST_CASE("dataset::write - accepts vector buffer")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "w");

    h5::dataset<float, 1> dataset = file.dataset<float, 1>("data/foo/bar");
    std::vector<float> const data = {1, 2, 3, 4, 5};
    dataset.write(data);

    CHECK(dataset.shape().dims[0] == data.size());
}

TEST_CASE("dataset::write - accepts custom vector buffer")
{
    temporary tmp;
    copy("data/sample.h5", tmp.filename);

    h5::file file(tmp.filename, "w");

    h5::dataset<double, 2> dataset = file.dataset<double, 2>("data/foo/bar");
    std::vector<triple_row> const data = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
    };
    dataset.write(data);

    CHECK(dataset.shape().dims[0] == data.size());
    CHECK(dataset.shape().dims[1] == 3);
}
