#include <iostream>
#include "../include/hnswlib/hnswlib/hnswlib.h"
#include "../utils/dataset.h"

// Function to parse command line arguments
int parse_arguments(int argc, char **argv, int &topk, int &train_elements, int &test_elements, int &dim, std::string &dataset_name)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--topk" && i + 1 < argc)
        {
            topk = std::atoi(argv[++i]);
        }
        else if (arg == "--train" && i + 1 < argc)
        {
            train_elements = std::atoi(argv[++i]);
        }
        else if (arg == "--test" && i + 1 < argc)
        {
            test_elements = std::atoi(argv[++i]);
        }
        else if (arg == "--dim" && i + 1 < argc)
        {
            dim = std::atoi(argv[++i]);
        }
        else if (arg == "--dataset" && i + 1 < argc)
        {
            dataset_name = argv[++i];
        }
        else
        {
            std::cerr << "Usage: " << argv[0] << " [--topk K] [--train N] [--test N] [--dim D] [--dataset DATASET_PATH]" << std::endl;
            std::cerr << "  --topk K: Number of nearest neighbors to find (default: 1)" << std::endl;
            std::cerr << "  --train N: Number of training elements (default: 100000)" << std::endl;
            std::cerr << "  --test N: Number of test elements (default: 10000)" << std::endl;
            std::cerr << "  --dim D: Dimension of the elements (default: 32)" << std::endl;
            std::cerr << "  --dataset DATASET_PATH: Path to the dataset file" << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int topk = 100;
    int train_elements = 1000;
    int test_elements = 100;
    int dim = 32;
    std::string dataset_name = ""; 
    
    // Parse command line arguments
    if (parse_arguments(argc, argv, topk, train_elements, test_elements, dim, dataset_name) != 0) {
        return 1;
    }
    if (dataset_name == "") {
        dataset_name = "random-euclidean-" + std::to_string(dim) + "-" + std::to_string(train_elements) + "-" + std::to_string(test_elements);
    }
    std::cout << "Dataset name: " << dataset_name << std::endl;
    
    std::string bin_file_name = "./data/" + dataset_name + ".bin";

    // Generate random data
    float *train_data = generate_random_data(dim, train_elements);
    float *test_data = generate_random_data(dim, test_elements);
    // Use Bruteforce to find ground truth neighbors and distances
    std::cout << "Using Bruteforce to find ground truth neighbors and distances" << std::endl;
    hnswlib::BruteforceSearch<float> *alg_bruteforce = new hnswlib::BruteforceSearch<float>(new hnswlib::L2Space(dim), train_elements);
    for (int i = 0; i < train_elements; i++) {
        alg_bruteforce->addPoint(train_data + i * dim, i);
    }
    std::cout << "Bruteforce index built" << std::endl;

    float *neighbors = new float[test_elements * topk];
    float *distances = new float[test_elements * topk];

    std::cout << "Searching for neighbors and distances" << std::endl;
    for (int i = 0; i < test_elements; i++) {
        std::priority_queue<std::pair<float, hnswlib::labeltype>> result = alg_bruteforce->searchKnn(test_data + i * dim, topk);
        
        for (int j = topk - 1; j >= 0; j--) {
            neighbors[i * topk + j] = result.top().second;
            distances[i * topk + j] = result.top().first;
            result.pop();
        }
    }
    std::cout << "Ground truth neighbors and distances found" << std::endl;

    delete alg_bruteforce;

    // Write data to binary file
    write_bin(bin_file_name, dim, train_elements, test_elements, topk, train_data, test_data, neighbors, distances);

}
