# Graphics Extensions Library (GXLIB)

## Overview

The Graphics Extensions Library (GXLIB) serves as a bridge between the crystallographic data structures of XLIB and the rendering capabilities of GLIB. It provides specialized visualization for crystallographic concepts like unit cells, atoms, bonds, and planes, enabling interactive exploration of crystal structures in 3D space.

## Key Features

- **Crystallographic Visualization**: Specialized rendering for crystallographic elements
- **Unit Cell Display**: Visualization of unit cells and lattices
- **Atom and Bond Rendering**: Visual representation of atoms and bonds
- **Symmetry Visualization**: Display of symmetry elements and operations
- **Interactive Selection**: Selection and manipulation of crystallographic objects
- **Labels and Annotations**: Labeling of atoms and other elements
- **Visual Styles**: Customizable visualization styles for different purposes

## Core Components

### Crystallographic Objects

- `TXAtom`: Visual representation of atoms, extending the crystallographic TCAtom
- `TXBond`: Visual representation of bonds, with rendering properties
- `TXPlane`: Visualization of crystallographic planes
- `TXAngle`: Visual representation of angles between atoms
- `TXLine`: Line visualization for various purposes
- `TXGrid`: Grid visualization for electron density and other volumetric data
- `TXBlob`: Blob visualization for molecular surfaces and voids

### Unit Cell and Lattice

- `TDUnitCell`: Visual representation of the unit cell
- `TXLattice`: Visual representation of the crystal lattice
- `TDRing`: Visualization of molecular rings
- `TXGrowLine`: Growth vector visualization
- `TXGrowPoint`: Growth point visualization

### Specialized Visualization

- `AtomLegend`: Legend for atom types and properties
- `TXTLS`: TLS (Translation, Libration, Screw) visualization
- `TDSphere`: Enhanced sphere visualization for atoms
- `TXReflection`: Visualization of reflection data
- `TXRestraint`: Visualization of crystallographic restraints

### Scene Management

- `TGXApp`: Application class bridging crystallographic and visual data
- `WGlScene`: Windowing-system specific scene implementation
- `TXGlLabels`: Enhanced labels for crystallographic objects

### State Management

- `OlxState`: State management for visualization settings
- `GXLibMacros`: Macros for common visualization operations

## Design Patterns

The GXLIB implements several design patterns:

- **Adapter Pattern**: Adapting crystallographic objects to visual objects
- **Decorator Pattern**: Adding visual properties to crystallographic objects
- **Strategy Pattern**: Different visualization strategies for different purposes
- **Observer Pattern**: Updating visuals when crystallographic data changes
- **Factory Pattern**: Creating visual representations of crystallographic objects

## Usage Examples

### Atom Visualization

```cpp
// Create a visual atom from a crystallographic atom
TCAtom* catom = &asymm_unit.GetAtom(0);
TXAtom* xatom = new TXAtom(*catom);

// Set visual properties
xatom->SetRadius(0.5);  // Visual radius
xatom->SetColor(1.0, 0.0, 0.0, 1.0);  // Red color
xatom->SetRenderingStyle(arsADPEllipsoid);  // Render as ADP ellipsoid

// Add to the scene
scene->AddObject(*xatom);
```

### Unit Cell Visualization

```cpp
// Create a visual unit cell
TDUnitCell* unitCell = new TDUnitCell();
unitCell->SetCell(10.0, 10.0, 10.0, 90.0, 90.0, 90.0);

// Configure appearance
unitCell->SetLineWidth(2.0);
unitCell->SetColor(0.0, 0.0, 0.0, 1.0);  // Black lines
unitCell->SetAxesVisible(true);

// Add to the scene
scene->AddObject(*unitCell);
```

### Electron Density Visualization

```cpp
// Create a grid for electron density
TXGrid* grid = new TXGrid();
grid->SetDimensions(50, 50, 50);
grid->SetCell(unitCell->GetCell());

// Load or calculate grid data
// ...

// Configure isosurface
grid->SetIsoLevel(1.2);
grid->SetPositiveColor(0.0, 0.0, 1.0, 0.7);  // Blue for positive
grid->SetNegativeColor(1.0, 0.0, 0.0, 0.7);  // Red for negative

// Add to the scene
scene->AddObject(*grid);
```

### Bond Visualization

```cpp
// Create a visual bond between two atoms
TXAtom* atom1 = scene->FindObject<TXAtom>("C1");
TXAtom* atom2 = scene->FindObject<TXAtom>("C2");

TXBond* bond = new TXBond(*atom1, *atom2);
bond->SetColor(0.5, 0.5, 0.5, 1.0);  // Gray color
bond->SetRadius(0.1);
bond->SetStyle(bsSolid);  // Solid bond style

// Add to the scene
scene->AddObject(*bond);
```

## Integration with XLIB and GLIB

GXLIB serves as an integration layer between XLIB and GLIB:

1. It takes crystallographic objects from XLIB (TCAtom, TSBond, etc.)
2. Creates visual representations (TXAtom, TXBond, etc.) with rendering properties
3. Uses GLIB's rendering capabilities to display these objects in 3D
4. Handles user interaction with the visual objects and updates the underlying crystallographic data

This integration enables a seamless workflow where changes to the crystallographic model are immediately reflected in the visualization and vice versa.

## File Organization

The GXLIB is primarily located in the `gxlib` directory and includes:

- Visual representation classes for crystallographic objects
- Specialized visualization utilities
- Integration classes between XLIB and GLIB
- State management for visualization

## Dependencies

The GXLIB depends on:

- XLIB for crystallographic data structures and operations
- GLIB for rendering capabilities
- SDL for fundamental utilities

## Extension Points

The GXLIB is designed to be extendable through:

- Implementation of new visualization styles
- Addition of specialized visualization for new crystallographic concepts
- Creation of new interactive manipulation tools
- Extension of selection and annotation capabilities
