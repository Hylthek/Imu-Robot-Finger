import csv
import sys

def analyze_time_differences(csv_file):
  times = []
  
  with open(csv_file, 'r') as f:
    reader = csv.reader(f)
    next(reader)  # Skip header
    for row in reader:
      if row:
        times.append(float(row[0]))
  
  if len(times) < 2:
    print("Not enough data points")
    return
  
  diffs = [times[i+1] - times[i] for i in range(len(times)-1)]
  
  print(f"Min diff: {min(diffs)}")
  print(f"Max diff: {max(diffs)}")
  print(f"Average diff: {sum(diffs) / len(diffs)}")

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print("Usage: python time_diff_stats.py <csv_file>")
    sys.exit(1)
  
  analyze_time_differences(sys.argv[1])