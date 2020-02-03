#include <cstdlib>
#include <exception>
#include <random>
#include <string>

#include <h5.hpp>

#include <catch.hpp>


namespace
{
    std::string make_random_name(int length)
    {
        static std::mt19937 random;

        static char const charset[] = "abcdefghijklmnopqrstuvwxyz";
        std::uniform_int_distribution<int> index{0, 25};

        std::string filename;
        for (int i = 0; i < length; i++) {
            filename += charset[index(random)];
        }
        return filename;
    }


    struct random_filename
    {
        std::string const name = "_data_" + make_random_name(10) + ".h5";

        random_filename() = default;

        ~random_filename()
        {
            std::system(("rm -f " + name).c_str());
        }

        random_filename(random_filename const&) = delete;
    };


    struct sample_data : random_filename
    {
        sample_data()
        {
            if (std::system(("cp data/sample.h5 " + name).c_str())) {
                throw std::exception();
            }
        }
    };
}


TEST_CASE("file - accepts mode string")
{
    SECTION("read-only")
    {
        // Opens existing file
        sample_data sample;
        h5::file file{sample.name, "r"};
    }

    SECTION("read-write")
    {
        // Opens existing file
        sample_data sample;
        h5::file file{sample.name, "r+"};
    }

    SECTION("create-truncate")
    {
        // Truncates existing file
        sample_data sample;
        h5::file file{sample.name, "w"};
    }

    SECTION("create-exclusive")
    {
        // Creates non-existent file
        random_filename data;
        h5::file file{data.name, "w-"};
    }
}
