#include <h5.hpp>

#include <catch.hpp>


TEST_CASE("shape - is equality comparable")
{
    h5::shape<3> const s1 = {1, 2, 3};
    h5::shape<3> const s2 = {1, 2, 3};
    h5::shape<3> const s3 = {1, 2, 9};

    CHECK(s1 == s2);
    CHECK(s1 != s3);
    CHECK(s3 != s1);
}

TEST_CASE("shape::size - returns total number of elements")
{
    h5::shape<3> const shape = {2, 3, 5};
    CHECK(shape.size() == 30);
}
