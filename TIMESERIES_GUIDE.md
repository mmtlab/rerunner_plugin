# Rerunner Plugin - Real-time Time Series Visualization

## Overview

This Rerunner plugin visualizes replay data as **real-time time series plots** using the [Rerun](https://www.rerun.io/) visualization framework. Data from your replay is streamed to Rerun and displayed live as you replay it.

## Key Features

### ✨ Real-time Time Series Visualization
- **Live plotting** of scalar values as they're replayed
- **Automatic timeline integration** - all data is timestamped
- **Multi-plot support** - visualize multiple signals simultaneously
- **Pan/zoom/inspect** capabilities in the Rerun viewer

### 📊 Built-in Statistical Analysis
- **Mean** - running average of signal values
- **Standard deviation** - measure of signal variability
- **Standard uncertainty** - uncertainty in the mean estimate
- **Auto-correlation (ACF)** - detect periodicity patterns
- **FFT analysis** - frequency domain visualization

### 💀 Skeleton Visualization
- **3D keypoint display** - pose estimation visualization
- **Confidence coloring** - visual feedback on detection confidence
- **Bone connections** - skeletal structure rendering
- **HPE 18-joint format support** - pre-configured for common datasets

## Setup

### Installation Requirements
```bash
# Install Rerun (if not already installed)
pip install rerun-sdk

# Build the plugin
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Plugin Configuration

The plugin is configured via JSON parameters. Key parameters:

```json
{
  "agent_name": "Replay Agent - Time Series Visualization",
  "time": "timestamp",              // Column name for time data
  
  "keypaths": [                     // Paths to plot as time series
    "pose/velocity_x",
    "sensors/accel_x"
  ],
  
  "acf_keypaths": [                 // Paths for ACF analysis
    "pose/velocity_x"
  ],
  
  "fft_keypaths": [                 // Paths for FFT analysis
    "pose/velocity_x"
  ],
  
  "window_size": 200,               // Statistics window size
  "blueprint": "timeseries_blueprint.json",  // Custom layout
  "parallelize": true               // Use multi-threading
}
```

## Data Flow

```
CSV/Data Source
      ↓
Plugin receives data (load_data callback)
      ↓
Set timeline timestamp (set_time_seconds)
      ↓
Extract numeric values from JSON paths
      ↓
Log to Rerun:
  - /timeseries/{keypath}      → Raw data points
  - /statistics/mean/{keypath} → Running mean
  - /statistics/stdev/{keypath}→ Running std dev
  - /analysis/acf/{keypath}    → Autocorrelation
  - /analysis/fft/{keypath}    → Frequency spectrum
  - /skeleton/*                → Pose visualization
      ↓
Rerun viewer updates in real-time
```

## Usage Examples

### Basic Setup with CSV Data

```python
import json
from pathlib import Path

# Load your data
csv_file = "SN000368420812_hpe_replay.csv"

# Configure the plugin
config = {
    "agent_name": "My Replay",
    "time": "timestamp",
    "keypaths": ["velocity_x", "velocity_y", "accel_x"],
    "window_size": 200
}

# Pass config to plugin
plugin.set_params(config)

# Stream data to plugin
for row in data:
    plugin.load_data(row)
```

### Keypath Specification

Keypaths use dot-notation to navigate JSON structure:

```python
# For data structure: {"pose": {"velocity": {"x": 1.5}}}
keypath = "pose/velocity/x"

# For array access: {"sensors": [10.2, 20.3, 30.1]}
keypath = "sensors/0"

# Supports both formats:
# - Dot notation: "pose.velocity.x"
# - Slash notation: "pose/velocity/x"
```

### Multi-channel Time Series

Plot multiple related signals:

```json
{
  "keypaths": [
    "body/acceleration/x",
    "body/acceleration/y", 
    "body/acceleration/z",
    "body/velocity/x",
    "body/velocity/y",
    "body/velocity/z"
  ]
}
```

In Rerun, each signal appears as a separate line in the time series plot, allowing you to correlate changes across channels.

### Skeleton Visualization with Time Series

Combine pose estimation with signal analysis:

```json
{
  "skeleton_type": "hpe_18",
  "skeleton_keypoint_paths": [
    "/pose/joints/nose",
    "/pose/joints/neck",
    "/pose/joints/shoulder_left",
    ...
  ],
  "keypaths": [
    "pose/velocity_x",
    "pose/acceleration_x"
  ]
}
```

The viewer will show both the 3D skeleton and the time series plots side-by-side.

## Visualization Interface

When you run a replay with this plugin:

1. **Time Series Panel** - Shows multiple line plots with:
   - X-axis: Time (seconds)
   - Y-axis: Signal values
   - Interactive legend to show/hide signals
   - Pan and zoom capabilities

2. **Statistics Panel** - Displays:
   - Running mean of each signal
   - Running standard deviation
   - Measurement uncertainty

3. **Skeleton Panel** - Shows:
   - 3D skeleton with keypoints (if pose data is available)
   - Confidence-based coloring
   - Bone connections

4. **Analysis Panel** - Contains:
   - ACF plots (autocorrelation function)
   - FFT plots (frequency spectrum)

## Advanced Features

### Custom Blueprint Layout

Create a custom blueprint JSON to arrange visualizations:

```json
{
  "layout": {
    "type": "grid",
    "columns": 2,
    "children": [
      {
        "path": "/timeseries",
        "display_name": "Raw Data"
      },
      {
        "path": "/statistics",
        "display_name": "Statistics"
      },
      {
        "path": "/analysis",
        "display_name": "Frequency Analysis"
      }
    ]
  }
}
```

Pass the blueprint path in config:
```json
{
  "blueprint": "path/to/your_blueprint.json"
}
```

### Parallel Processing

For high-frequency data, enable parallel statistics computation:

```json
{
  "parallelize": true  // Uses available CPU cores
}
```

### Adaptive Window Size

The window size for statistics affects the precision:

```json
{
  "window_size": 200  // Use 200 samples for ACF/FFT computation
}
```

## Troubleshooting

### Time Series Not Appearing
- Check that `time` parameter matches your timestamp column name
- Verify keypaths exist in your data (check debug output)
- Ensure numeric values are being extracted (not strings)

### Performance Issues
- Reduce window_size for faster computation
- Enable `parallelize: true`
- Reduce the number of keypaths being logged

### Data Not Aligned Temporally
- Verify timestamp values increase monotonically
- Check that all data points have valid timestamps

## Dependencies

- **Rerun SDK** - For visualization
- **nlohmann/json** - JSON handling
- **BS::thread_pool** - Optional parallel processing
- **pugg** - Plugin system

## Output Paths

Data is organized in the Rerun viewer as:

```
root/
├── timeseries/         # Raw time series data
├── statistics/         # Statistical metrics
│   ├── mean/
│   ├── stdev/
│   └── std_uncertainty/
├── analysis/           # Advanced analysis
│   ├── acf/           # Autocorrelation functions
│   └── fft/           # Frequency spectra
├── skeleton/          # Pose visualization
│   ├── keypoints
│   └── bones
└── debug/             # Debug information
```

## Performance Notes

- **Typical data rate**: 100-1000 samples/second
- **Visualization latency**: <100ms
- **Memory usage**: Proportional to window_size
- **CPU usage**: Minimal for serial processing, scales with parallelize enabled

## References

- [Rerun Documentation](https://docs.rerun.io/)
- [Time Series Best Practices](https://docs.rerun.io/howto/render-2d-line-plots)
- [Plugin Development Guide](../README.md)
