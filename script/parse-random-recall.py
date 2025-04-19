import os
import re
import csv
import argparse
import tarfile
from collections import defaultdict


def parse_log_file(file_content):
    recall = None # Initialize recall

    # Handle both string content (from file) and bytes content (from tar)
    if isinstance(file_content, bytes):
        lines = file_content.decode("utf-8").split("\n")
    else:
        lines = file_content.split("\n")

    for line in lines:
        # Extract recall using regex to handle Recall@K format
        if match := re.search(r"Recall@\d+: ([\d.]+)%", line):
             recall = float(match.group(1))


    return recall # Return only recall


def process_file(file_path, pattern, results):
    if file_path.endswith(".log"):
        match = re.search(pattern, os.path.basename(file_path))
        if match:
            # Use more descriptive variable names
            dimension = int(match.group(1))
            topk = int(match.group(2))

            with open(file_path, "r") as f:
                file_content = f.read()
                recall = parse_log_file(file_content) # Get only recall

                key = (dimension, topk) # Use dimension and topk as key

                if recall is not None: # Store recall if found
                    results[key]["recalls"].append(recall)


def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(
        description="Parse log files and generate CSV results"
    )
    parser.add_argument(
        "input_path", help="Path to the tar.gz file or directory containing log files"
    )
    args = parser.parse_args()

    # Extract CPU model from input path and remove .tar.gz suffix if present
    base_name = os.path.basename(args.input_path.rstrip("/"))
    if base_name.endswith(".tar.gz"):
        base_name = base_name[:-7]  # Remove .tar.gz suffix
    cpu_model = base_name.split("-")[1] if "-" in base_name else "unknown"
    # Initialize defaultdict to include only recalls list
    results = defaultdict(lambda: {"recalls": []})

    # Regular expression to extract dimension and topk from filename
    pattern = r"sse42-random-euclidean-(\d+)-1000-100-top(\d+)-run\d+\.log"

    # Process input based on its type
    if args.input_path.endswith(".tar.gz"):
        try:
            with tarfile.open(args.input_path, "r:gz") as tar:
                for member in tar.getmembers():
                    if member.isfile() and member.name.endswith(".log"):
                         match = re.search(pattern, os.path.basename(member.name))
                         if match:
                            dimension = int(match.group(1))
                            topk = int(match.group(2))
                            f = tar.extractfile(member)
                            if f:
                                file_content = f.read()
                                recall = parse_log_file(file_content)
                                key = (dimension, topk)
                                if recall is not None:
                                    results[key]["recalls"].append(recall)
        except tarfile.ReadError:
             print(f"Error: Could not read tar file {args.input_path}")
             return
        except FileNotFoundError:
             print(f"Error: Tar file not found at {args.input_path}")
             return

    elif os.path.isdir(args.input_path):
        # Process directory
        for root, _, files in os.walk(args.input_path):
            for file in files:
                file_path = os.path.join(root, file)
                process_file(file_path, pattern, results)
    else:
        print(f"Error: Input path {args.input_path} is not a valid directory or .tar.gz file.")
        return


    # Sort results by dimension and topk
    sorted_results = sorted(results.items(), key=lambda x: (x[0][0], x[0][1]))

    # Save results to CSV
    csv_filename = f"recall-random-M8-ef200.csv"
    with open(csv_filename, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        # Write header including only recall
        writer.writerow(
            [
                "dimension",
                "topk",
                "avg_recall(%)", # Only recall header
            ]
        )

        # Write data
        for (dim, topk), data in sorted_results: # Changed 'times' to 'data' for clarity
            # Calculate averages, handle potential empty lists
            avg_recall = (
                sum(data["recalls"]) / len(data["recalls"])
                if data["recalls"]
                else 0 # Calculate average recall
            )
            # Write only dim, topk, and avg_recall
            writer.writerow(
                [dim, topk, f"{avg_recall:.3f}"] # Write only recall value
            )

    print(f"Results have been saved to {csv_filename}")


if __name__ == "__main__":
    main()
