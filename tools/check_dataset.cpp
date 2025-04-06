#include "../include/hnswlib/hnswlib/hnswlib.h"
#include "../utils/dataset.h"

// Function to parse command line arguments
int parse_arguments(int argc, char **argv, std::string &dataset_name, int &topk)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--topk" && i + 1 < argc)
        {
            topk = std::atoi(argv[++i]);
        }
        else if (arg == "--dataset" && i + 1 < argc)
        {
            dataset_name = argv[++i];
        }
        else
        {
            std::cerr << "Usage: " << argv[0] << " [--dataset DATASET_PATH] [--topk K]" << std::endl;
            std::cerr << "  --dataset DATASET_PATH: Path to the dataset file" << std::endl;
            std::cerr << "  --topk K: Number of nearest neighbors to find (default: 1)" << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    std::string dataset_path = "./data/random-xs-32-euclidean.bin";
    int topk = 1;
    if (parse_arguments(argc, argv, dataset_path, topk) != 0) {
        return 1;
    }
    int dim, train_elements, test_elements, dataset_topk;
    float *train_data, *test_data, *neighbors, *distances;
    read_bin(dataset_path, dim, train_elements, test_elements, dataset_topk, train_data, test_data, neighbors, distances);
    std::cout << "Dataset dimension: " << dim << std::endl;
    std::cout << "Train elements: " << train_elements << std::endl;
    std::cout << "Test elements: " << test_elements << std::endl;
    std::cout << "Dataset topk: " << dataset_topk << std::endl;


    // Use Bruteforce to check the dataset
    std::cout << "Using Bruteforce to check the dataset with topk = " << topk << std::endl;
    if (topk > dataset_topk) {
        std::cerr << "Error: topk must be less than dataset_topk" << std::endl;
        return 1;
    }
    hnswlib::BruteforceSearch<float> *alg_bruteforce = new hnswlib::BruteforceSearch<float>(new hnswlib::L2Space(dim), train_elements);
    for (int i = 0; i < train_elements; i++) {
        alg_bruteforce->addPoint(train_data + i * dim, i);
    }
    std::cout << "Bruteforce index built" << std::endl;

    std::cout << "Checking the dataset" << std::endl;
    long long total_correct = 0;
    long long total_comparisons = test_elements * topk;
    // Check the dataset
    for (int i = 0; i < test_elements; i++) {
        std::priority_queue<std::pair<float, hnswlib::labeltype>> result = alg_bruteforce->searchKnn(test_data + i * dim, topk);
        
        std::vector<hnswlib::labeltype> found_labels;
        std::vector<int> ground_truth;
        for (int j = 0; j < topk; j++)
        {
            ground_truth.push_back(static_cast<int>(neighbors[i * dataset_topk + j]));
        }

        while (!result.empty())
        {
            found_labels.push_back(result.top().second);
            result.pop();
        }

        for (const auto &found : found_labels)
        {
            if (std::find(ground_truth.begin(), ground_truth.end(), found) != ground_truth.end())
            {
                total_correct++;
            }
        }
    }

    float recall = static_cast<float>(total_correct) / total_comparisons;
    std::cout << "Total correct: " << total_correct << std::endl;
    std::cout << "Total comparisons: " << total_comparisons << std::endl;
    std::cout << "Recall: " << recall << std::endl;

    std::cout << "Done checking the dataset" << std::endl;

    return 0;
}
