# Component Interactions in Olex2

## Overview

Olex2's architecture consists of several layers of components that work together to create a complete crystallographic application. This document explains how these components interact and the data flow between them, providing a comprehensive picture of how Olex2 functions as an integrated system.

## Component Dependency Hierarchy

The components in Olex2 have a hierarchical dependency structure:

```
┌───────────────────────────────────────────┐
│              Application Layer            │
│                   (OLEX)                  │
│                      ▲                    │
├──────────────────────┼────────────────────┤
│        Crystallographic Visualization     │
│                  (GXLIB)                  │
│                      ▲                    │
├───────────────┬──────┼──────┬─────────────┤
│   Graphics    │ Crystallography│   Core   │
│    Library    │    Library     │ Utilities │
│    (GLIB)     │    (XLIB)      │   (SDL)   │
│       ▲       │       ▲        │     ▲     │
└───────┼───────┴───────┼────────┴─────┼─────┘
        │               │              │
        └───────────────┴──────────────┘
```

Each component depends on components below it in the hierarchy:
- OLEX depends on all other components
- GXLIB depends on GLIB, XLIB, and SDL
- GLIB depends on SDL
- XLIB depends on SDL
- SDL is the foundational component with no internal dependencies

## Key Interaction Points

### Between SDL and Other Components

SDL provides foundational utilities used by all other components:
- Data structures (lists, trees, maps) used throughout the application
- File I/O facilities used by file format parsers in XLIB
- Exception handling used for error management across all components
- Memory management with smart pointers used throughout the application
- Mathematical utilities used for crystallographic calculations in XLIB

### Between XLIB and GXLIB

XLIB provides crystallographic data that GXLIB visualizes:
- `TCAtom` objects from XLIB are wrapped by `TXAtom` objects in GXLIB
- Symmetry operations from XLIB are used by GXLIB to generate symmetry-equivalent visual elements
- Unit cell information from XLIB is used by GXLIB for unit cell visualization
- Molecular connectivity from XLIB is used by GXLIB for bond visualization
- Electron density data from XLIB is visualized as grid surfaces by GXLIB

### Between GLIB and GXLIB

GLIB provides rendering capabilities that GXLIB uses for crystallographic visualization:
- `TGlPrimitive` objects from GLIB are used by GXLIB to render atoms, bonds, etc.
- Scene management from GLIB is extended by GXLIB for crystallographic scene organization
- Materials and lighting from GLIB are used by GXLIB for visual appearance
- Selection and picking from GLIB are used by GXLIB for interactive selection

### Between GXLIB and OLEX

GXLIB provides crystallographic visualization that OLEX integrates into the application:
- Visual crystallographic objects from GXLIB are rendered in the main application window
- Interactive manipulation in OLEX updates the underlying GXLIB objects
- Visual styles in GXLIB are controlled by UI elements in OLEX
- Selection events from GXLIB are processed by OLEX for UI updates

### Between XLIB and OLEX

XLIB provides crystallographic functionality that OLEX exposes through its interface:
- File format handling in XLIB is used by OLEX for file I/O
- Refinement operations in XLIB are exposed through OLEX's interface
- Analysis functions in XLIB are presented in OLEX's UI
- Crystallographic data from XLIB is displayed in information panels in OLEX

## Data Flow Examples

### Loading a Crystal Structure

1. User initiates file open operation in OLEX
2. OLEX uses SDL's file I/O facilities to access the file
3. XLIB parsers read and interpret the crystallographic data
4. XLIB creates internal representations (TCAtom, TSBond, etc.)
5. GXLIB creates visual representations (TXAtom, TXBond, etc.)
6. GLIB renders these visual objects in the 3D scene
7. OLEX updates the UI to reflect the loaded structure

### Structure Refinement

1. User initiates refinement in OLEX
2. OLEX passes parameters to XLIB
3. XLIB performs refinement calculations
4. XLIB updates internal crystallographic data
5. GXLIB updates visual representations
6. GLIB re-renders the updated scene
7. OLEX displays refinement results and updates UI

### Interactive Atom Manipulation

1. User selects and moves an atom in the 3D view
2. GLIB handles picking and initial mouse interaction
3. GXLIB identifies the corresponding TXAtom
4. GXLIB updates the visual position
5. XLIB updates the corresponding TCAtom coordinates
6. OLEX updates property panels and information displays
7. Additional dependent visualizations (bonds, angles) are updated

## Communication Mechanisms

Several mechanisms facilitate communication between components:

### Observer Pattern

The observer pattern is used extensively:
- XLIB objects notify observers when crystallographic data changes
- GXLIB objects observe XLIB objects and update visualizations accordingly
- OLEX observes both XLIB and GXLIB to update the UI

### Command Pattern

The command pattern enables actions across components:
- OLEX creates command objects that encapsulate operations
- Commands may involve multiple components (e.g., both XLIB and GXLIB)
- Commands support undo/redo functionality across components

### Event System

An event system propagates notifications:
- Selection events from GLIB/GXLIB are passed to OLEX
- File change events from XLIB are passed to OLEX and GXLIB
- Error events from any component are passed to OLEX for display

### Direct Function Calls

For tight integration, components make direct function calls:
- GXLIB calls XLIB functions to access crystallographic data
- OLEX calls GXLIB functions to control visualization
- All components call SDL functions for utility operations

## Integration Challenges and Solutions

### Maintaining Consistency

Ensuring consistency between crystallographic data (XLIB) and visualization (GXLIB):
- Observer pattern keeps visual representations synchronized with data
- Transactions group related changes to maintain consistency
- Validation ensures crystallographic correctness

### Performance Optimization

Balancing performance across components:
- Lazy updates defer visualization changes until necessary
- Level-of-detail mechanisms in GXLIB reduce rendering load
- Threading isolates heavy calculations in XLIB from UI responsiveness

### Extensibility

Maintaining extensibility across component boundaries:
- Factory patterns allow new object types in any component
- Interface-based design enables component substitution
- Plugin architecture allows extending functionality at multiple levels

## Conclusion

The interaction between Olex2's components creates a cohesive system where:
- The foundational utilities of SDL support all other components
- The crystallographic functionality of XLIB provides the scientific core
- The rendering capabilities of GLIB enable visualization
- The integration in GXLIB connects crystallographic data with visualization
- The application layer in OLEX brings everything together in a user-friendly interface

This architecture enables Olex2 to provide a powerful yet accessible tool for crystallographers, with a clean separation of concerns that facilitates maintenance and extension.
