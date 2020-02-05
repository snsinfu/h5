#ifndef INCLUDED_UTILS_HPP
#define INCLUDED_UTILS_HPP

#include <cstdlib>
#include <exception>
#include <random>
#include <string>


namespace detail
{
    // Generates a random alphabetical name of given length.
    inline std::string make_random_name(int length)
    {
        // This does not need to be secure.
        static std::mt19937 random;

        static char const charset[] = "abcdefghijklmnopqrstuvwxyz";
        std::uniform_int_distribution<int> index{0, 25};

        std::string name;
        for (int i = 0; i < length; i++) {
            name += charset[index(random)];
        }
        return name;
    }
}


// Generates random temporary path. The path gets removed when the object goes
// out of scope.
struct temporary
{
    // Random HDF5 filename.
    std::string const filename = "_data_" + detail::make_random_name(8) + ".h5";

    temporary() = default;

    ~temporary()
    {
        (void) std::system(("rm -f " + filename).c_str());
    }
};


// Copy file from `src` to `dest`.
inline void copy(std::string const& src, std::string const& dest)
{
    if (std::system(("cp " + src + " " + dest).c_str())) {
        throw std::exception();
    }
}


#endif
