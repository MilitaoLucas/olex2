# Olex2 User Guide

This guide provides information on common workflows and operations in Olex2, aimed at crystallographers using the software for structure solution, refinement, and analysis.

## Getting Started

### Installation

Olex2 is available for Windows, Linux, and macOS platforms. To install:

1. Download the appropriate installer from the [Olex2 website](https://www.olexsys.org/olex2/)
2. Run the installer and follow the on-screen instructions
3. For Linux, you may need to install additional dependencies:
   ```bash
   sudo apt-get install libgl1-mesa-glx libglu1-mesa libxrandr2 libxss1 libxcursor1 libxcomposite1 libasound2 libxi6 libxtst6
   ```

### First Launch

When launching Olex2 for the first time:

1. The program will check for necessary components and download them if missing
2. You'll be prompted to set up initial preferences
3. The main interface will appear with a blank structure

### Interface Overview

The Olex2 interface consists of several key areas:

- **3D View**: Central area displaying the crystal structure
- **Command Line**: Bottom area for entering text commands
- **Information Panels**: Right side showing structure information
- **Toolbars**: Top and left sides with common operations
- **Context Menus**: Right-click in the 3D view for context-specific options

## Basic Operations

### Loading a Structure

To load an existing structure:

1. Select **File > Open** or press **Ctrl+O**
2. Navigate to your structure file (CIF, INS, RES, etc.)
3. Select the file and click **Open**

Alternatively, use the command:
```
reap filename.ext
```

### Saving a Structure

To save the current structure:

1. Select **File > Save** or press **Ctrl+S** for the current format
2. Select **File > Save As** to choose a different format or location

Alternatively, use commands:
```
# Save in current format
save

# Save as CIF
write cif filename.cif

# Save as SHELX format
write res filename.res
```

### Structure Visualization

Control the appearance of the structure:

- **Rotate**: Click and drag in the 3D view
- **Zoom**: Scroll wheel or right-click and drag vertically
- **Pan**: Middle-click and drag

Change display styles:
1. Select atoms or bonds
2. Right-click and choose **Style**
3. Select the desired style

Alternatively, use commands:
```
# Change atom style to ellipsoid
style ellipsoid

# Change bond style to stick
style -b stick
```

### Selection

Select atoms and other objects:

- **Single Click**: Select a single atom
- **Ctrl+Click**: Add to selection
- **Shift+Click**: Select atoms in a rectangular area
- **Alt+Click**: Select a molecule

Use selection commands:
```
# Select all carbon atoms
sel $C

# Select atoms by label
sel C1 C2 C3

# Select atoms by element and geometry
sel $C(ring)
```

## Structure Solution and Refinement

### Structure Solution

To solve a structure from diffraction data:

1. Load your HKL and INS files
2. Go to **Work > Structure Solution**
3. Choose a solution method (e.g., ShelXT, ShelXS)
4. Click **Solve**

Alternatively, use the command:
```
solve
```

### Structure Refinement

To refine a structure:

1. Go to **Work > Structure Refinement**
2. Set refinement options
3. Click **Refine**

Alternatively, use commands:
```
# Basic refinement
refine

# More detailed refinement
refine -l anis
```

### Handling Disorder

To model disorder:

1. Select the disordered atoms
2. Right-click and select **Disorder > Split**
3. Adjust occupancies in the property panel

Use commands for more control:
```
# Split an atom
split C1

# Set occupancy
occu C1A 0.7 C1B 0.3
```

## Analysis and Validation

### Geometric Analysis

Measure geometric parameters:

1. Select two atoms for a distance, three for an angle, or four for a torsion
2. Right-click and select **Geometry > Measure**

Use commands:
```
# Measure distance
dist C1 C2

# List all bonds to an atom
bonds C1
```

### Structure Validation

Check the structure for issues:

1. Go to **Work > Validation**
2. Review the checklist of potential issues
3. Address each issue as needed

Validate using commands:
```
# Check for issues
validate

# Check specific aspects
validation_check adp
```

### Structure Report

Generate a report of the structure:

1. Go to **Work > Report**
2. Choose the report type and options
3. Click **Generate**

Use commands:
```
# Generate HTML report
report -f html

# Generate CIF report
report -f cif
```

## Advanced Features

### Molecular Packing

View packing arrangements:

1. Go to **View > Packing**
2. Choose packing options
3. Click **Apply**

Use commands:
```
# Show unit cell packing
pack

# Pack with specific parameters
pack -r 1 1 1
```

### Electron Density

Visualize electron density maps:

1. Go to **Maps > Density Map**
2. Set contour level and display options
3. Click **Show Map**

Use commands:
```
# Show difference map
fmap -d

# Show electron density map
fmap -f
```

### Constraints and Restraints

Apply crystallographic constraints and restraints:

1. Select atoms to constrain/restrain
2. Right-click and select **Constrain/Restrain**
3. Choose the constraint/restraint type and parameters

Use commands:
```
# Apply AFIX constraint
afix 66 $C(ring)

# Apply DFIX restraint
dfix 1.54 0.01 C1 C2
```

## Scripting and Automation

### Using the Command Line

The command line provides direct access to Olex2's functionality:

1. Press **Ctrl+L** to focus on the command line
2. Type a command and press **Enter**
3. Use **Up/Down** arrows to navigate command history

### Python Scripts

Run Python scripts for automation:

1. Go to **Work > Scripts**
2. Select or create a script
3. Click **Run**

Example Python script:
```python
import olex
from olexpy import utils

# Get all carbon atoms
carbons = [a for a in olex.xf.au.atoms() if a.type() == 'C']

# Calculate average C-C bond length
c_c_bonds = []
for a in carbons:
    for b in a.bonds():
        if b.atom(1).type() == 'C' and b.atom(0).type() == 'C':
            c_c_bonds.append(b.length())

if c_c_bonds:
    avg_length = sum(c_c_bonds) / len(c_c_bonds)
    olex.m("Average C-C bond length: {:.3f} Å".format(avg_length))
```

### Batch Processing

Process multiple structures:

1. Create a Python script for batch processing
2. Use the command line to run the script with multiple files

Example batch script:
```python
import olex
import os
import sys

def process_file(filename):
    # Load file
    olex.xf.reap(filename)
    
    # Perform operations
    olex.m("Processing " + filename)
    olex.c("refine")
    
    # Save results
    output = os.path.splitext(filename)[0] + "_refined.cif"
    olex.c("write cif " + output)

# Process all files given as arguments
for arg in sys.argv[1:]:
    process_file(arg)
```

## Troubleshooting

### Common Issues

#### Structure Won't Refine Properly

- Check for missing or misidentified atoms
- Verify space group assignment
- Check for twinning or disorder
- Examine residual electron density maps

#### Graphics Problems

- Update graphics drivers
- Reduce graphics quality in preferences
- Check OpenGL compatibility

#### Crashes or Freezes

- Check for available updates
- Verify input file integrity
- Check for sufficient disk space and memory

### Getting Help

- **In-App Help**: Press **F1** or go to **Help > Contents**
- **Online Documentation**: Visit the [Olex2 website](https://www.olexsys.org/olex2/docs/)
- **Community Forum**: Post questions on the [Olex2 forum](https://olex2.sourceforge.net/forum/)
- **Contact Support**: Email [support@olex2.org](mailto:support@olex2.org)

## Tips and Tricks

### Keyboard Shortcuts

- **Ctrl+S**: Save
- **Ctrl+O**: Open
- **Ctrl+Z**: Undo
- **Ctrl+Y**: Redo
- **Ctrl+L**: Focus command line
- **F1**: Help
- **F5**: Refresh view
- **Space**: Center on selection

### Customization

Customize the interface:

1. Go to **Tools > Preferences**
2. Adjust settings according to your needs
3. Click **Apply** or **OK**

### Performance Optimization

For large structures:

- Reduce display complexity in **View > Display Options**
- Use simplified atom/bond styles
- Disable real-time updating during refinement
