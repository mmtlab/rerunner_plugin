#!/usr/bin/env python3
"""
Example script demonstrating real-time time series visualization 
with the Rerunner plugin.

This shows how to replay data from a CSV file and visualize it 
as a time series in real-time using Rerun.
"""

import json
import csv
import time
from pathlib import Path

# Configuration for the plugin
PLUGIN_CONFIG = {
    "agent_name": "Replay Agent - Time Series Visualization",
    "time": "timestamp",  # Column name for time data
    
    # Keypaths for time series data - these will be plotted as line charts
    "keypaths": [
        "pose/velocity_x",
        "pose/velocity_y", 
        "pose/velocity_z",
        "sensors/accel_x",
        "sensors/accel_y",
        "sensors/accel_z"
    ],
    
    # Keypaths for autocorrelation function analysis
    "acf_keypaths": [
        "pose/velocity_x",
        "sensors/accel_x"
    ],
    
    # Keypaths for FFT/frequency analysis
    "fft_keypaths": [
        "pose/velocity_x",
        "sensors/accel_x"
    ],
    
    # Window size for statistics (samples)
    "window_size": 200,
    
    # Blueprint file path for custom visualization layout
    "blueprint": "timeseries_blueprint.json",
    
    # Enable parallel processing
    "parallelize": True,
    
    # Skeleton configuration (if using pose data)
    "skeleton_type": "hpe_18",
    "enable_bones": True,
    "enable_keypoints": True,
    "use_confidence_colors": True,
    "min_confidence": 0.1
}

def load_csv_data(csv_file: str):
    """Load data from CSV file and yield rows with timestamp."""
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            yield row

def convert_row_to_dict(row: dict) -> dict:
    """Convert CSV row to nested dictionary structure."""
    result = {}
    
    for key, value in row.items():
        if value == '':
            continue
        
        try:
            # Try to convert to float
            float_val = float(value)
        except ValueError:
            # Keep as string if not a number
            float_val = value
        
        # Parse nested keys (e.g., "pose.velocity_x" -> result["pose"]["velocity_x"])
        parts = key.split('.')
        current = result
        for part in parts[:-1]:
            if part not in current:
                current[part] = {}
            current = current[part]
        current[parts[-1]] = float_val
    
    return result

def simulate_replay(csv_file: str, output_file: str = "replay_output.jsonl", 
                   playback_speed: float = 1.0):
    """
    Simulate replay by reading CSV and generating output in JSONL format.
    
    Args:
        csv_file: Path to input CSV file
        output_file: Path to output JSONL file
        playback_speed: Playback speed multiplier (1.0 = real-time)
    """
    if not Path(csv_file).exists():
        print(f"CSV file not found: {csv_file}")
        return
    
    print(f"Loading data from {csv_file}...")
    
    rows = list(load_csv_data(csv_file))
    if not rows:
        print("No data found in CSV file")
        return
    
    print(f"Found {len(rows)} data points")
    print(f"Writing replay data to {output_file}...")
    
    with open(output_file, 'w') as out_f:
        for idx, row in enumerate(rows):
            data = convert_row_to_dict(row)
            
            # Ensure timestamp exists
            if 'timestamp' not in data:
                data['timestamp'] = float(idx)
            
            json_line = json.dumps({
                "timestamp": idx,
                "data": data
            })
            out_f.write(json_line + '\n')
            
            if (idx + 1) % 100 == 0:
                print(f"  Processed {idx + 1}/{len(rows)} rows...")
    
    print(f"Replay data saved to {output_file}")
    print("\nExample usage in Python:")
    print(f"  # Load the data")
    print(f"  with open('{output_file}') as f:")
    print(f"    for line in f:")
    print(f"      data = json.loads(line)")
    print(f"      # Pass 'data' to the plugin")

def print_config():
    """Print the plugin configuration."""
    print("\n" + "="*60)
    print("RERUNNER PLUGIN - TIME SERIES VISUALIZATION CONFIG")
    print("="*60)
    print(json.dumps(PLUGIN_CONFIG, indent=2))
    print("="*60 + "\n")

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python replay_example.py <csv_file> [output_file] [playback_speed]")
        print("\nExample:")
        print("  python replay_example.py SN000368420812_hpe_replay.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "replay_output.jsonl"
    playback_speed = float(sys.argv[3]) if len(sys.argv) > 3 else 1.0
    
    print_config()
    simulate_replay(csv_file, output_file, playback_speed)
