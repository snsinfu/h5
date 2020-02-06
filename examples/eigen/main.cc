#include <iostream>
#include <random>

#include <Eigen/Dense>

#include <h5.hpp>


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

    file.dataset<h5::f32, 2>("eigen/matrix").write(
        &matrix(0, 0),
        {1000, 1000},
        {.compression = 1, .scaleoffset = 3}
    );
    std::clog << "Matrix is written to dump.h5\n";

    // Read back into another matrix.
    auto dataset = file.dataset<h5::f32, 2>("eigen/matrix");
    auto shape = dataset.shape();

    Matrix buffer(Matrix::Index(shape.dims[0]), Matrix::Index(shape.dims[1]));
    dataset.read(&buffer(0, 0), shape);

    std::clog << "Matrix is read back from the file\n";
}
