import os
import csv
import argparse
import re
from gem5_metrics import metrics

# Removed parse_parent_directory_name function

def parse_directory_name(dir_name):
    """Parse the directory name to extract configuration parameters."""
    # Example: m5out-hifive-random-euclidean-8-1000-100-top1
    pattern = r'm5out-hifive-(\w+)-(\w+)-(\d+)-(\d+)-(\d+)-top(\d+)'
    match = re.match(pattern, dir_name)
    if match:
        return {
            'workload_type': 'hifive',
            'dataset_type': match.group(1),
            'distance_type': match.group(2),
            'dimension': match.group(3),
            'num_points': match.group(4),
            'num_queries': match.group(5),
            'top_k': match.group(6)
        }
    return None

def extract_metrics(stats_file):
    """Extract metrics from stats.txt file."""
    extracted_data = {}
    with open(stats_file, "r") as file:
        for line in file:
            for key in metrics.keys():
                # Ensure the key is found as a whole word or at the beginning of the line followed by space/tab
                # Use regex for more precise matching if needed, e.g., r'\b' + re.escape(key) + r'\b'
                # Simple check for now:
                if line.startswith(key + ' ') or (' ' + key + ' ' in line):
                     # Split carefully, handle potential multiple spaces
                    parts = line.split()
                    try:
                        # Find the index of the key and get the next element as value
                        idx = parts.index(key)
                        if idx + 1 < len(parts):
                            value = parts[idx+1]
                            # Basic check if value looks like a number (int or float)
                            if re.match(r'^-?\d+(\.\d+)?(e[+-]?\d+)?$', value, re.IGNORECASE):
                                extracted_data[metrics[key]] = value
                            # Handle cases like 'NaN' or 'inf' if necessary
                            elif value.lower() in ['nan', 'inf', '-inf']:
                                extracted_data[metrics[key]] = value
                            # else: # Optional: log or handle non-numeric values if needed
                            #    print(f"Warning: Non-numeric value found for {key}: {value}")
                        # else: # Optional: log or handle if key is the last word
                        #    print(f"Warning: No value found after key {key} in line: {line.strip()}")
                    except ValueError:
                        # Key might be a substring of another word, ignore if simple 'in' check was used
                        # Or handle error if specific format is expected
                        pass
                    except IndexError:
                         # Handle cases where split doesn't produce expected parts
                        pass
                        # print(f"Warning: Could not parse value for key {key} in line: {line.strip()}")

    return extracted_data


def get_required_fields():
    """Get all required fields from directory names and metrics."""
    config_fields = [
        # 'vlen', 'elen', # Removed vlen and elen
        'workload_type', 'dataset_type', 'distance_type',
        'dimension', 'num_points', 'num_queries', 'top_k'
    ]
    metric_fields = list(metrics.values())
    return config_fields + metric_fields

def validate_data(data, required_fields):
    """Validate data has all required fields and fill missing ones with None or raise error."""
    validated_data = {}
    missing_fields = []
    for field in required_fields:
        if field not in data:
            # Option 1: Fill with None (or NaN, or a specific placeholder)
            # validated_data[field] = None
            # missing_fields.append(field)

            # Option 2: Raise error (current behavior)
            raise ValueError(f"Missing required field: {field} in data derived from directory/stats")
        else:
            validated_data[field] = data[field]

    # If using Option 1, you might want to log the missing fields:
    # if missing_fields:
    #     print(f"Warning: Missing fields {missing_fields} filled with None for data: {data}")

    return validated_data


def process_results(results_dir):
    """Process all results directories and generate a single CSV file."""
    all_results = []
    required_fields = get_required_fields()

    # Removed the check for parent_dir_name starting with 'hifive-output'
    # The loop below directly looks for 'm5out-hifive-*' directories within results_dir

    for item in os.listdir(results_dir):
        item_path = os.path.join(results_dir, item)
        # Check if it's a directory and matches the m5out pattern
        if os.path.isdir(item_path) and item.startswith('m5out-hifive-'):
            config = parse_directory_name(item)
            if config:
                stats_file = os.path.join(item_path, 'stats.txt')
                if os.path.exists(stats_file):
                    try:
                        metrics_data = extract_metrics(stats_file)
                        # Combine config and metrics
                        combined_data = {**config, **metrics_data}
                        # Validate data
                        validated_data = validate_data(combined_data, required_fields)
                        all_results.append(validated_data)
                    except ValueError as e:
                        print(f"Skipping directory {item} due to validation error: {e}")
                    except Exception as e:
                        print(f"Skipping directory {item} due to unexpected error during processing: {e}")

                else:
                    print(f"Warning: stats.txt not found in {item_path}")
            # else: # Optional: log if a directory starts with m5out-hifive- but doesn't parse correctly
                # print(f"Warning: Could not parse configuration from directory name: {item}")

    if all_results:
        # Write all results to a single CSV file in the results directory
        output_file = os.path.join(results_dir, 'hifive_performance_summary.csv') # Changed filename
        try:
            with open(output_file, 'w', newline='') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=required_fields)
                writer.writeheader()
                writer.writerows(all_results)
            print(f"Generated summary file: {output_file}")
        except IOError as e:
            print(f"Error writing CSV file {output_file}: {e}")
    else:
        print("No valid results found to generate summary file.")


def main():
    parser = argparse.ArgumentParser(
        description="Parse HiFive performance data from m5out directories" # Updated description
    )
    parser.add_argument(
        "--resultsdir",
        type=str,
        required=True, # Make it required if it doesn't have a sensible default relative path
        # default="../results/hifive-output", # Or provide a more specific default
        help="Directory containing the m5out-hifive-* result subdirectories (e.g., path/to/hifive-output)" # Updated help text
    )
    args = parser.parse_args()

    if not os.path.isdir(args.resultsdir):
        print(f"Error: Results directory '{args.resultsdir}' not found.")
        return

    process_results(args.resultsdir)

if __name__ == "__main__":
    main()
