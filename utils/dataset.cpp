#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>

#ifdef HDF5_ENABLED
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
#endif

void read_bin(const std::string &file_name, int &dim, int &train_elements, int &test_elements, int &dataset_topk, float *&train_data, float *&test_data, float *&neighbors, float *&distances)
{
    // Check if file exists
    std::ifstream file_check(file_name);
    if (!file_check.good())
    {
        std::cerr << "Error: File " << file_name << " does not exist" << std::endl;
        exit(1);
    }
    file_check.close();
    std::ifstream binary_file(file_name, std::ios::binary);
    binary_file.read((char *)&dim, sizeof(int));
    binary_file.read((char *)&train_elements, sizeof(int));
    binary_file.read((char *)&test_elements, sizeof(int));
    binary_file.read((char *)&dataset_topk, sizeof(int));

    train_data = new float[dim * train_elements];
    test_data = new float[dim * test_elements];
    neighbors = new float[dataset_topk * test_elements];
    distances = new float[dataset_topk * test_elements];

    binary_file.read((char *)train_data, dim * train_elements * sizeof(float));
    binary_file.read((char *)test_data, dim * test_elements * sizeof(float));
    binary_file.read((char *)neighbors, dataset_topk * test_elements * sizeof(float));
    binary_file.read((char *)distances, dataset_topk * test_elements * sizeof(float));
    binary_file.close();
    std::cout << "Read data from " << file_name << " with " << train_elements << " train elements and " << test_elements << " test elements\n";
    std::cout << "Dataset Dimension: " << dim << std::endl;
    std::cout << "Dataset topk: " << dataset_topk << std::endl;
}

void write_bin(const std::string &file_name, int dim, int train_elements, int test_elements, int dataset_topk, float *train_data, float *test_data, float *neighbors, float *distances)
{
    // The binary file format is:
    // - 4 bytes: dimension of the elements
    // - 4 bytes: number of training elements
    // - 4 bytes: number of test elements
    // - 4 bytes: number of neighbors
    // - dim * train_elements * 4 bytes: training data
    // - dim * test_elements * 4 bytes: test data
    // - topk * test_elements * 4 bytes: neighbors
    // - topk * test_elements * 4 bytes: distances
    std::ofstream binary_file(file_name, std::ios::binary);
    binary_file.write((char *)&dim, sizeof(int));
    binary_file.write((char *)&train_elements, sizeof(int));
    binary_file.write((char *)&test_elements, sizeof(int));
    binary_file.write((char *)&dataset_topk, sizeof(int));

    binary_file.write((char *)train_data, dim * train_elements * sizeof(float));
    binary_file.write((char *)test_data, dim * test_elements * sizeof(float));
    binary_file.write((char *)neighbors, dataset_topk * test_elements * sizeof(float));
    binary_file.write((char *)distances, dataset_topk * test_elements * sizeof(float));
}

float *generate_random_data(int dim, int num_elements, int seed = 1)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<> distrib_real;
    float *data = new float[dim * num_elements];
    for (int i = 0; i < dim * num_elements; i++)
    {
        data[i] = distrib_real(rng);
    }
    std::cout << "Generated random data with " << num_elements << " elements and " << dim << " dimensions\n";
    return data;
}
