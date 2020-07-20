#include <iostream>
#include <random>

#include <Eigen/Dense>

#include <h5.hpp>


namespace h5
{
    // Adapt Eigen::Matrix as a two-dimensional buffer.
    template<typename T>
    struct buffer_traits<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
    {
        using buffer_type = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        using value_type = T;

        static constexpr int rank = 2;

        static h5::shape<rank> shape(buffer_type const& buffer)
        {
            auto const rows = static_cast<std::size_t>(buffer.rows());
            auto const cols = static_cast<std::size_t>(buffer.cols());
            return {rows, cols};
        }

        static value_type const* data(buffer_type const& buffer)
        {
            return &buffer(0, 0);
        }

        static value_type* data(buffer_type& buffer)
        {
            return &buffer(0, 0);
        }

        static void reshape(buffer_type& buffer, h5::shape<rank> const& shape)
        {
            using index_type = typename buffer_type::Index;
            buffer.resize(
                static_cast<index_type>(shape.dims[0]),
                static_cast<index_type>(shape.dims[1])
            );
        }
    };
}


int main()
{
    // Create some random matrix.
    using Matrix = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    Matrix matrix(1000, 1000);

    std::mt19937 random;
    std::normal_distribution<double> normal;
    for (int i = 0; i < matrix.rows(); i++) {
        for (int j = 0; j < matrix.cols(); j++) {
            matrix(i, j) = normal(random);
        }
    }

    matrix = Matrix(matrix.transpose() * matrix);

    // Store the matrix in an HDF5 file.
    h5::file file("dump.h5", "w");

    file.dataset<h5::f32, 2>("eigen/matrix").write(matrix);

    std::clog << "Matrix is written to dump.h5\n";
    std::clog << "sum = " << matrix.sum() << '\n';

    // Read back into another matrix.
    Matrix buffer;

    file.dataset<h5::f32, 2>("eigen/matrix").read_fit(buffer);

    std::clog << "Matrix is read back from the file\n";
    std::clog << "sum = " << buffer.sum() << '\n';
}
