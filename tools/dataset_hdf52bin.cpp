#include <iostream>
#include <fstream>
#include <H5Cpp.h>
#include "../utils/dataset.h"

float *read_hdf5(const std::string &file_name, const std::string &dataset_name, int &num_elements, int &dim)
{
    H5::H5File file(file_name, H5F_ACC_RDONLY);
    H5::DataSet dataset = file.openDataSet(dataset_name);
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t dims_out[2];
    dataspace.getSimpleExtentDims(dims_out, NULL);
    num_elements = dims_out[0];
    dim = dims_out[1];
    float *data = new float[dim * num_elements];
    dataset.read(data, H5::PredType::NATIVE_FLOAT);
    std::cout << "Read data from " << file_name << " with " << num_elements << " elements and " << dim << " dimensions\n";
    return data;
}

int main()
{
    int dim;              // Dimension of the elements
    int train_elements = 100;
    int test_elements = 100;
    int topk = 1;

    // Prompt user for input HDF5 file name
    std::string file_name;
    std::cout << "Enter the input HDF5 file name (default: ./data/random-xs-32-euclidean.hdf5): ";
    std::getline(std::cin, file_name);
    if (file_name.empty())
    {
        file_name = "./data/random-xs-32-euclidean.hdf5";
    }
    std::cout << "Input file name: " << file_name << "\n";

    // Read data from HDF5 file
    float *train_data = read_hdf5(file_name, "train", train_elements, dim);
    float *test_data = read_hdf5(file_name, "test", test_elements, dim);
    float *neighbors = read_hdf5(file_name, "neighbors", test_elements, topk);
    float *distances = read_hdf5(file_name, "distances", test_elements, topk);

    // Generate output binary file name based on input file name
    std::string binary_file_name = file_name.substr(0, file_name.find_last_of('.')) + ".bin";

    // Store the data in a binary file
    std::cout << "Output binary file: " << binary_file_name << "\n";
    write_bin(binary_file_name, dim, train_elements, test_elements, topk, train_data, test_data, neighbors, distances);

    // Test reading the binary file
    std::ifstream binary_file_read(binary_file_name, std::ios::binary);
    int dim_read;
    int train_elements_read;
    int test_elements_read;
    int topk_read;
    float *train_data_read, *test_data_read, *neighbors_read, *distances_read;
    read_bin(binary_file_name, dim_read, train_elements_read, test_elements_read, topk_read, train_data_read, test_data_read, neighbors_read, distances_read);

    // Compare the data
    bool is_same = true;
    for (int i = 0; i < dim * train_elements; i++)
    {
        if (train_data[i] != train_data_read[i])
        {
            is_same = false;
            break;
        }
    }
    for (int i = 0; i < dim * test_elements; i++)
    {
        if (test_data[i] != test_data_read[i])
        {
            is_same = false;
            break;
        }
    }
    for (int i = 0; i < topk * test_elements; i++)
    {
        if (neighbors[i] != neighbors_read[i])
        {
            is_same = false;
            break;
        }
        if (distances[i] != distances_read[i])
        {
            is_same = false;
            break;
        }
    }
    if (is_same)
    {
        std::cout << "Data is the same\n";
    }
    else
    {
        std::cout << "Data is different\n";
    }

    // Clean up
    delete[] train_data;
    delete[] test_data;
    delete[] neighbors;
    delete[] distances;
    delete[] train_data_read;
    delete[] test_data_read;
    delete[] neighbors_read;
    delete[] distances_read;
}
