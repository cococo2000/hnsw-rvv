import os
import csv
import argparse
from gem5_metrics import metrics

# Set up command-line argument parsing
parser = argparse.ArgumentParser(description='Extract metrics from stats.txt file')
parser.add_argument('--statsdir', type=str, default='../results/RVVCore/m5out',
                    help='Directory containing the stats.txt file')
args = parser.parse_args()

results_dir = args.statsdir
stats_file_path = os.path.join(results_dir, 'stats.txt')
output_csv_path = os.path.join(results_dir, 'summary_stats.csv')

def extract_metrics(stats_file):
    extracted_data = {}
    with open(stats_file, 'r') as file:
        for line in file:
            for key in metrics.keys():
                if key in line:
                    value = line.split()[1]
                    extracted_data[key] = value
    return extracted_data

def write_to_csv(data, output_file):
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['metric', 'value'])
        for key, value in data.items():
            writer.writerow([metrics[key], value])

if __name__ == "__main__":
    metrics_data = extract_metrics(stats_file_path)
    write_to_csv(metrics_data, output_csv_path)
