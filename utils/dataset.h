#pragma once
#include <string>

/**
 * @brief Read data from an HDF5 file
 * @param file_name Path to the HDF5 file
 * @param dataset_name Name of the dataset within the HDF5 file
 * @param[out] num_elements Number of elements in the dataset
 * @param[out] dim Dimension of each element
 * @return Pointer to the read data array
 */
#ifdef HDF5_ENABLED
float *read_hdf5(const std::string &file_name, const std::string &dataset_name, int &num_elements, int &dim);
#endif

/**
 * @brief Read data from a binary file containing training and test datasets
 * @param file_name Path to the binary file
 * @param[out] dim Dimension of each element
 * @param[out] train_elements Number of training elements
 * @param[out] test_elements Number of test elements  
 * @param[out] dataset_topk Number of nearest neighbors stored for each test element
 * @param[out] train_data Pointer to training data array
 * @param[out] test_data Pointer to test data array
 * @param[out] neighbors Pointer to nearest neighbors array
 * @param[out] distances Pointer to distances array
 */
void read_bin(const std::string &file_name, int &dim, int &train_elements, int &test_elements, int &dataset_topk, float *&train_data, float *&test_data, float *&neighbors, float *&distances);

/**
 * @brief Write data to a binary file
 * @param file_name Path to the binary file
 * @param dim Dimension of each element
 * @param train_elements Number of training elements
 * @param test_elements Number of test elements
 * @param dataset_topk Number of nearest neighbors stored for each test element
 */
void write_bin(const std::string &file_name, int dim, int train_elements, int test_elements, int dataset_topk, float *train_data, float *test_data, float *neighbors, float *distances);

/**
 * @brief Generate random data points
 * @param dim Dimension of each element
 * @param num_elements Number of elements to generate
 * @param seed Seed for the random number generator
 * @return Pointer to the generated data array
 */
float *generate_random_data(int dim, int num_elements, int seed = 1);
