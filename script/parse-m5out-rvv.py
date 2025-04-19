import os
import csv
import argparse
import re
from gem5_metrics import metrics

def parse_parent_directory_name(dir_name):
    """Parse the parent directory name to extract vlen and elen parameters."""
    # Example: output-vlen256-elen64
    pattern = r'(?:rv|rvv)?-?output-vlen(\d+)-elen(\d+)'
    match = re.match(pattern, dir_name)
    if match:
        return {
            'vlen': match.group(1),
            'elen': match.group(2)
        }
    return None

def parse_directory_name(dir_name):
    """Parse the directory name to extract configuration parameters."""
    # Example: m5out-rv-random-euclidean-8-1000-100-top1
    pattern = r'm5out-(rv|rvv|hifive)-(\w+)-(\w+)-(\d+)-(\d+)-(\d+)-top(\d+)'
    match = re.match(pattern, dir_name)
    if match:
        return {
            'workload_type': match.group(1),
            'dataset_type': match.group(2),
            'distance_type': match.group(3),
            'dimension': match.group(4),
            'num_points': match.group(5),
            'num_queries': match.group(6),
            'top_k': match.group(7)
        }
    return None

def extract_metrics(stats_file):
    """Extract metrics from stats.txt file."""
    extracted_data = {}
    with open(stats_file, "r") as file:
        for line in file:
            for key in metrics.keys():
                if key in line:
                    value = line.split()[1]
                    extracted_data[metrics[key]] = value
    return extracted_data

def get_required_fields():
    """Get all required fields from directory names and metrics."""
    config_fields = [
        'vlen', 'elen',
        'workload_type', 'dataset_type', 'distance_type',
        'dimension', 'num_points', 'num_queries', 'top_k'
    ]
    metric_fields = list(metrics.values())
    return config_fields + metric_fields

def validate_data(data, required_fields):
    """Validate data has all required fields and raise error if any field is missing."""
    validated_data = {}
    for field in required_fields:
        if field not in data:
            raise ValueError(f"Missing required field: {field}")
        validated_data[field] = data[field]
    return validated_data

def process_results(results_dir):
    """Process all results directories and generate a single CSV file."""
    all_results = []
    required_fields = get_required_fields()
    
    for root, dirs, files in os.walk(results_dir):
        # Look for directories matching the pattern output-vlen*-elen*
        parent_dir = os.path.basename(root)
        parent_config = parse_parent_directory_name(parent_dir)
        
        if parent_config:
            # Process each subdirectory
            for dir_name in dirs:
                if dir_name.startswith('m5out-'):
                    config = parse_directory_name(dir_name)
                    if config:
                        stats_file = os.path.join(root, dir_name, 'stats.txt')
                        if os.path.exists(stats_file):
                            metrics_data = extract_metrics(stats_file)
                            # Combine parent config, subdirectory config and metrics
                            combined_data = {**parent_config, **config, **metrics_data}
                            # Validate data and raise error if any field is missing
                            validated_data = validate_data(combined_data, required_fields)
                            # if validated_data['workload_type'] == 'rv':
                            #     continue
                            all_results.append(validated_data)
    
    if all_results:
        # Write all results to a single CSV file in the results directory
        output_file = os.path.join(results_dir, 'all_performance_summary.csv')
        with open(output_file, 'w', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=required_fields)
            writer.writeheader()
            writer.writerows(all_results)
        print(f"Generated summary file: {output_file}")

def main():
    parser = argparse.ArgumentParser(
        description="Parse performance data from results directories"
    )
    parser.add_argument(
        "--resultsdir",
        type=str,
        default="../results",
        help="Directory containing the results directories"
    )
    args = parser.parse_args()
    
    process_results(args.resultsdir)

if __name__ == "__main__":
    main() 
