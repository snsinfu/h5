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

    bool operator==(triple_row const& r1, triple_row const& r2)
    {
        return r1.x == r2.x && r1.y == r2.y && r1.z == r2.z;
    }

    bool operator!=(triple_row const& r1, triple_row const& r2)
    {
        return !(r1 == r2);
    }
}

namespace h5
{
    // Vector of triple_row objects as a two-dimensional n-by-3 array.
    template<>
    struct buffer_traits<std::vector<triple_row>>
    {
        static_assert(
            sizeof(triple_row) == 3 * sizeof(double), "unexpected padding"
        );

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


TEST_CASE("dataset::write/read - accepts vector buffer")
{
    temporary tmp;

    h5::file file(tmp.filename, "w");

    // Write

    h5::dataset<float, 1> dataset = file.dataset<float, 1>("data");
    std::vector<float> const data = {1, 2, 3, 4, 5};
    dataset.write(data);

    CHECK(dataset.shape().dims[0] == data.size());

    // Read

    SECTION("dataset::read - reads custom vector buffer")
    {
        std::vector<float> buffer(dataset.shape().dims[0]);
        dataset.read(buffer);

        CHECK(buffer == data);
    }

    SECTION("dataset::read_fit - reads custom vector buffer")
    {
        std::vector<float> buffer;
        dataset.read_fit(buffer);

        CHECK(buffer == data);
    }
}

TEST_CASE("dataset::write/read - accepts custom vector buffer")
{
    temporary tmp;

    h5::file file(tmp.filename, "w");

    // Write

    h5::dataset<double, 2> dataset = file.dataset<double, 2>("data");
    std::vector<triple_row> const data = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
    };
    dataset.write(data);

    CHECK(dataset.shape().dims[0] == data.size());
    CHECK(dataset.shape().dims[1] == 3);

    // Read

    SECTION("dataset::read - reads custom vector buffer")
    {
        std::vector<triple_row> buffer(dataset.shape().dims[0]);
        dataset.read(buffer);

        CHECK(buffer == data);
    }

    SECTION("dataset::read_fit - reads custom vector buffer")
    {
        std::vector<triple_row> buffer;
        dataset.read_fit(buffer);

        CHECK(buffer == data);
    }
}
