import os
import csv
import argparse
from gem5_metrics import metrics

# Set up command-line argument parsing
parser = argparse.ArgumentParser(
    description="Merge metrics from all stats.txt files in results directory"
)
parser.add_argument(
    "--resultsdir",
    type=str,
    default="../results",
    help="Directory containing the results directories",
)
args = parser.parse_args()

results_dir = args.resultsdir

def extract_metrics(stats_file):
    extracted_data = {}
    with open(stats_file, "r") as file:
        for line in file:
            for key in metrics.keys():
                if key in line:
                    value = line.split()[1]
                    extracted_data[key] = value
    return extracted_data

def write_to_csv(data, output_file):
    with open(output_file, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        # Write header
        headers = ["metric"] + list(data.keys())
        writer.writerow(headers)
        # Write data
        for metric in metrics.keys():
            row = [metrics[metric]]
            for dir_name in data.keys():
                row.append(data[dir_name].get(metric, "N/A"))
            writer.writerow(row)

def merge_results(results_dir):
    for root, dirs, files in os.walk(results_dir):
        if root == results_dir:
            for dir_name in dirs:
                dir_path = os.path.join(root, dir_name)
                merged_data = {}
                for sub_dir_name in os.listdir(dir_path):
                    sub_dir_path = os.path.join(dir_path, sub_dir_name)
                    stats_file_path = os.path.join(sub_dir_path, "stats.txt")
                    if os.path.exists(stats_file_path):
                        metrics_data = extract_metrics(stats_file_path)
                        merged_data[sub_dir_name] = metrics_data
                output_csv_path = os.path.join(dir_path, "merged_summary_stats.csv")
                write_to_csv(merged_data, output_csv_path)

if __name__ == "__main__":
    merge_results(results_dir)
