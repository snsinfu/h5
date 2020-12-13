# Cookbook

- [Incrementally write cellular automaton states to disk](#incrementally-write-cellular-automaton-states-to-disk)

## Incrementally write cellular automaton states to disk

```c++
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

#include <h5.hpp>


int main()
{
    constexpr std::size_t size = 50;
    constexpr long num_steps = 100000;

    h5::file output("output.h5", "w");

    std::vector<unsigned> state(size);
    std::vector<unsigned> new_state(size);

    // Random initialization.
    std::mt19937 random;
    std::uniform_int_distribution<unsigned> uniform(0, 1);
    std::generate(state.begin(), state.end(), [&] { return uniform(random); });

    // Open a two-dimensional dataset with data type u8 (8-bit unsigned int).
    auto state_dataset = output.dataset<h5::u8, 2>("state_history");

    // Start incremental writing to the dataset. Here {size} specifies that
    // the shape of the dataset is (infinite, size). The option scaleoffset = 0
    // is effective to compress integral dataset better. When this option is
    // set, the lowest compression factor compression = 1 is sufficient.
    auto state_stream = state_dataset.stream_writer(
        {size}, {.compression = 1, .scaleoffset = 0}
    );

    // Save initial state.
    state_stream.write(state);

    for (long step = 0; step < num_steps; step++) {
        // Rule 105.
        for (std::size_t i = 0; i < size; i++) {
            unsigned sum = state[i];
            sum += state[(i + size - 1) % size];
            sum += state[(i + 1) % size];
            new_state[i] = 1 ^ (sum & 1);
        }
        state.assign(new_state.begin(), new_state.end());

        // Save state after every update.
        state_stream.write(state);
    }
}
```

Run the code and inspect the output file. You see 100000-by-50 matrix stored
in the output HDF5 file.

```console
$ c++ -std=c++14 -O2 -isystem include -o example example.cc -lhdf5
$ ./example
$ h5ls -rv output.h5
Opened "output.h5" with sec2 driver.
/                        Group
    Location:  1:96
    Links:     1
/state_history           Dataset {100000/Inf, 50/50}
    Location:  1:800
    Links:     1
    Chunks:    {492, 50} 24600 bytes
    Storage:   5000000 logical bytes, 631781 allocated bytes, 791.41% utilization
    Filter-0:  scaleoffset-6 OPT {2, 0, 24600, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    Filter-1:  shuffle-2 OPT {1}
    Filter-2:  deflate-1 OPT {1}
    Type:      native unsigned char
```
