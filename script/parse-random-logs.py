import os
import re
import csv
import argparse
import tarfile
from collections import defaultdict

def parse_log_file(file_content):
    index_time = None
    search_time = None
    
    # Handle both string content (from file) and bytes content (from tar)
    if isinstance(file_content, bytes):
        lines = file_content.decode('utf-8').split('\n')
    else:
        lines = file_content.split('\n')
    
    for line in lines:
        if 'Time to create index:' in line:
            index_time = float(line.split(':')[1].strip().split()[0])
        elif 'Time to search:' in line:
            search_time = float(line.split(':')[1].strip().split()[0])
    
    return index_time, search_time

def process_file(file_path, pattern, results):
    if file_path.endswith('.log'):
        match = re.search(pattern, os.path.basename(file_path))
        if match:
            workload_type = match.group(1)
            dimension = int(match.group(2))
            topk = int(match.group(3))
            
            with open(file_path, 'r') as f:
                file_content = f.read()
                index_time, search_time = parse_log_file(file_content)
                
                if index_time is not None:
                    results[(workload_type, dimension, topk)]['index_times'].append(index_time)
                if search_time is not None:
                    results[(workload_type, dimension, topk)]['search_times'].append(search_time)

def process_tar(tar_path, pattern, results):
    with tarfile.open(tar_path, 'r:gz') as tar:
        for member in tar.getmembers():
            if member.name.endswith('.log'):
                match = re.search(pattern, member.name)
                if match:
                    workload_type = match.group(1)
                    dimension = int(match.group(2))
                    topk = int(match.group(3))
                    
                    file_content = tar.extractfile(member).read()
                    index_time, search_time = parse_log_file(file_content)
                    
                    if index_time is not None:
                        results[(workload_type, dimension, topk)]['index_times'].append(index_time)
                    if search_time is not None:
                        results[(workload_type, dimension, topk)]['search_times'].append(search_time)

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Parse log files and generate CSV results')
    parser.add_argument('input_path', help='Path to the tar.gz file or directory containing log files')
    args = parser.parse_args()
    
    # Extract CPU model from input path and remove .tar.gz suffix if present
    base_name = os.path.basename(args.input_path.rstrip('/'))
    if base_name.endswith('.tar.gz'):
        base_name = base_name[:-7]  # Remove .tar.gz suffix
    cpu_model = base_name.split('-')[1] if '-' in base_name else 'unknown'
    results = defaultdict(lambda: {'index_times': [], 'search_times': []})
    
    # Regular expression to extract workload_type and dimension from filename
    # Corrected the pattern to use alternation correctly
    pattern = r'(rvv|rv|base|avx|avx512|sse|sse4|hifive)-random-euclidean-(\d+)-1000-100-top(\d+)(-run\d+)?\.log'
    
    # Process input based on its type
    if args.input_path.endswith('.tar.gz'):
        process_tar(args.input_path, pattern, results)
    else:
        # Process directory
        for root, _, files in os.walk(args.input_path):
            for file in files:
                file_path = os.path.join(root, file)
                process_file(file_path, pattern, results)
    
    # Sort results by workload_type, dimension and topk
    sorted_results = sorted(results.items(), key=lambda x: (x[0][0], x[0][1], x[0][2]))
    
    # Save results to CSV
    csv_filename = f'results-{cpu_model}.csv'
    with open(csv_filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        writer.writerow(['workload_type', 'dimension', 'topk', 'avg_index_time(ms)', 'avg_search_time(ms)'])
        
        # Write data
        for (workload_type, dim, topk), times in sorted_results:
            avg_index = sum(times['index_times']) / len(times['index_times'])
            avg_search = sum(times['search_times']) / len(times['search_times'])
            writer.writerow([workload_type, dim, topk, f"{avg_index:.3f}", f"{avg_search:.3f}"])
    
    print(f"Results have been saved to {csv_filename}")

if __name__ == "__main__":
    main()
