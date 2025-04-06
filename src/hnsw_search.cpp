#include "../include/hnswlib/hnswlib/hnswlib.h"
#include "../utils/dataset.h"
#include <iostream>
#include <chrono>
#include <iomanip>

hnswlib::HierarchicalNSW<float> *create_index(hnswlib::SpaceInterface<float> &space, int dim, int train_elements, int M, int ef_construction, float *data)
{
    hnswlib::HierarchicalNSW<float> *alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space, train_elements, M, ef_construction);
    for (int i = 0; i < train_elements; i++)
    {
        alg_hnsw->addPoint(data + i * dim, i);
    }
    return alg_hnsw;
}

void do_search(hnswlib::HierarchicalNSW<float> *alg_hnsw, float *data, int query_elements, int dim, int topk)
{
    for (int i = 0; i < query_elements; i++)
    {
        std::priority_queue<std::pair<float, hnswlib::labeltype>> result = alg_hnsw->searchKnn(data + i * dim, topk);
    }
}

float calculate_recall(hnswlib::HierarchicalNSW<float> *alg_hnsw, float *query_data, float *neighbors, int num_queries, int dim, int topk, int dataset_topk)
{
    long long total_correct = 0;
    long long total_comparisons = num_queries * topk;

    for (int i = 0; i < num_queries; i++)
    {
        std::priority_queue<std::pair<float, hnswlib::labeltype>> result = alg_hnsw->searchKnn(query_data + i * dim, topk);
        std::vector<hnswlib::labeltype> found_labels;

        while (!result.empty())
        {
            found_labels.push_back(result.top().second);
            result.pop();
        }

        std::vector<int> ground_truth;
        for (int j = 0; j < topk; j++)
        {
            if (neighbors == nullptr && topk == 1)
            {
                ground_truth.push_back(i);
            }
            else if (neighbors != nullptr)
            {
                ground_truth.push_back(static_cast<int>(neighbors[i * dataset_topk + j]));
            }
            else
            {
                std::cerr << "Error: No ground truth data provided" << std::endl;
                exit(1);
            }
        }

        for (const auto &found : found_labels)
        {
            if (std::find(ground_truth.begin(), ground_truth.end(), found) != ground_truth.end())
            {
                total_correct++;
            }
        }
    }

    std::cout << "Total correct: " << total_correct << std::endl;
    std::cout << "Total comparisons: " << total_comparisons << std::endl;
    float recall = static_cast<float>(total_correct) / total_comparisons;
    return recall;
}

void evaluate_search_quality(hnswlib::HierarchicalNSW<float> *alg_hnsw, float *query_data, float *neighbors, int num_queries, int dim, int topk, int dataset_topk)
{
    std::cout << "--------- Search Quality Evaluation ---------" << std::endl;
    float recall = calculate_recall(alg_hnsw, query_data, neighbors, num_queries, dim, topk, dataset_topk);
    std::cout << "Number of queries: " << num_queries << std::endl;
    std::cout << "Top-K: " << topk << std::endl;
    std::cout << "Recall@" << topk << ": " << recall * 100 << "%" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
}

// Function to parse command line arguments
int parse_arguments(int argc, char **argv, int &topk, int &M, int &ef_construction, std::string &dataset_name)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--topk" && i + 1 < argc)
        {
            topk = std::atoi(argv[++i]);
        }
        else if (arg == "--M" && i + 1 < argc)
        {
            M = std::atoi(argv[++i]);
        }
        else if (arg == "--ef" && i + 1 < argc)
        {
            ef_construction = std::atoi(argv[++i]);
        }
        else if (arg == "--dataset" && i + 1 < argc)
        {
            dataset_name = argv[++i];
        }
        else
        {
            std::cerr << "Usage: " << argv[0] << " [--topk K] [--M M] [--ef EF] [--dataset DATASET_PATH]" << std::endl;
            std::cerr << "  --topk K: Number of nearest neighbors to find (default: 1)" << std::endl;
            std::cerr << "  --M M: HNSW graph connectivity parameter (default: 64)" << std::endl;
            std::cerr << "  --ef EF: HNSW construction parameter (default: 500)" << std::endl;
            std::cerr << "  --dataset DATASET_PATH: Path to the dataset file" << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int dim = 0;
    int train_elements = 0;
    int test_elements = 0;
    int dataset_topk = 0;
    float *train_data = nullptr;
    float *test_data = nullptr;
    float *neighbors = nullptr;
    float *distances = nullptr;

    // Parameters for HNSW
    int M = 8;                 // Parameter affecting internal connectivity and memory consumption
    int ef_construction = 200; // Controls search/indexing speed trade-off
    int topk = 1;              // Default topk value
    std::string dataset_name = "/home/zxge/VDB/hnsw/data/random-xs-32-euclidean.bin";

    // Parse command line arguments
    if (parse_arguments(argc, argv, topk, M, ef_construction, dataset_name) != 0)
    {
        return 1;
    }

    // Set precision for floating-point output
    std::cout << std::fixed << std::setprecision(3);

    // Initialize time variables
    std::chrono::high_resolution_clock::time_point start, end;
    long long duration;

    // -------------- Read data --------------
    std::cout << "--------- Reading data ---------" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    // // Read data from HDF5 file
    // std::string file_name = "../../data/random-xs-32-euclidean.hdf5";
    // std::string dataset_name = "train";
    // train_data = read_hdf5(file_name, dataset_name, train_elements, dim);
    // end = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // std::cout << ">>> Time to read HDF5 file: " << duration / 1000.0 << " ms\n";

    // // Generate random data
    // dim = 8;              // Dimension of the elements
    // train_elements = 1000;  // Maximum number of elements
    // train_data = generate_random_data(dim, train_elements);
    // end = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // std::cout << ">>> Time to generate random data: " << duration / 1000.0 << " ms\n";

    // Read data from binary file
    read_bin(dataset_name, dim, train_elements, test_elements, dataset_topk, train_data, test_data, neighbors, distances);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << ">>> Time to read binary file: " << duration / 1000.0 << " ms\n";

    // Check if requested topk is valid
    if (topk > dataset_topk && test_data != nullptr)
    {
        std::cerr << "Error: Requested topk (" << topk << ") is larger than dataset topk (" << dataset_topk << ")\n";
        return 1;
    }

    // -------------- Create index --------------
    std::cout << "--------- Creating index ---------" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    hnswlib::L2Space space(dim);
    hnswlib::HierarchicalNSW<float> *alg_hnsw = create_index(space, dim, train_elements, M, ef_construction, train_data);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << ">>> Time to create index: " << duration / 1000.0 << " ms\n";

    // -------------- Search --------------
    std::cout << "--------- Searching ---------" << std::endl;
    if (test_data == nullptr)
    {
        test_data = train_data;
        test_elements = train_elements;
    }
    start = std::chrono::high_resolution_clock::now();
    // %%%%%%%% Time measurement %%%%%%%%
    do_search(alg_hnsw, test_data, test_elements, dim, topk);
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // %%%%%%%% Recall measurement %%%%%%%%
    // evaluate_search_quality(alg_hnsw, test_data, neighbors, test_elements, dim, topk, dataset_topk);
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << ">>> Time to search: " << duration / 1000.0 << " ms\n";

    delete alg_hnsw;
    return 0;
}
