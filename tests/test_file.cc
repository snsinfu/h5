#include <h5.hpp>

#include <catch.hpp>

#include "utils.hpp"


TEST_CASE("file - accepts mode string")
{
    SECTION("read-only")
    {
        // Opens existing file
        temporary tmp;
        copy("data/sample.h5", tmp.filename);
        h5::file(tmp.filename, "r");
    }

    SECTION("read-write")
    {
        // Opens existing file
        temporary tmp;
        copy("data/sample.h5", tmp.filename);
        h5::file(tmp.filename, "r+");
    }

    SECTION("create-truncate")
    {
        // Truncates existing file
        temporary tmp;
        copy("data/sample.h5", tmp.filename);
        h5::file(tmp.filename, "w");
    }

    SECTION("create-exclusive")
    {
        // Creates non-existent file
        temporary tmp;
        h5::file(tmp.filename, "w-");
    }
}
