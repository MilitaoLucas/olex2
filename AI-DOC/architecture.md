# Olex2 Architecture Overview

## Introduction

Olex2 is built with a modular architecture that separates concerns between different components. This document provides a high-level overview of the architecture, how the components interact, and the design principles employed.

## Architectural Layers

Olex2 follows a layered architecture:

```
┌───────────────────────────────────────────┐
│              Application Layer            │
│                   (olex)                  │
├───────────────────────────────────────────┤
│        Crystallographic Visualization     │
│                  (gxlib)                  │
├───────────────┬───────────────┬───────────┤
│   Graphics    │Crystallography│    Core   │
│    Library    │    Library    │  Utilities│
│    (glib)     │    (xlib)     │    (sdl)  │
└───────────────┴───────────────┴───────────┘
```

### Core Utilities Layer (SDL)

The Software Development Library (SDL) provides foundational utilities and data structures used throughout the application:

- Data structures (trees, lists, dictionaries)
- Memory management
- File I/O and serialization
- Exception handling
- Threading utilities
- Mathematical algorithms

### Crystallography Library (XLIB)

The Crystallography Library implements domain-specific functionality:

- Crystallographic calculations
- Symmetry operations
- Space group handling
- CIF file parsing and generation
- Atom, bond, and molecular structure representation
- Refinement algorithms
- Crystallographic constraints and restraints

### Graphics Library (GLIB)

The Graphics Library provides visualization capabilities:

- OpenGL-based rendering
- Scene management
- Materials and textures
- Lighting and shading
- Camera and view control
- Primitive rendering (spheres, cylinders, etc.)

### Crystallographic Visualization (GXLIB)

This layer bridges the crystallography and graphics components:

- Specialized rendering for crystallographic elements
- Unit cell visualization
- Atom, bond, and plane rendering
- Labels and annotations
- Interactive selection and manipulation

### Application Layer (OLEX)

The Application Layer implements the user interface and application-specific functionality:

- GUI implementation
- User interaction handling
- Application state management
- File handling and project management
- Dialog boxes and controls
- Update mechanism

## Key Architectural Concepts

### Object Hierarchy

Olex2 employs an extensive object hierarchy with careful use of inheritance and polymorphism to represent crystallographic concepts. Key base classes include:

- `IOlxObject`: Base for objects that can be identified and managed
- `AReferencible`: Base for objects that can be referenced
- `APerishable`: Base for objects that require cleanup when no longer needed
- `TDataItem`: Base for objects that can store and retrieve data

### Observer Pattern

The software uses the Observer pattern extensively to maintain consistency between the model and views:

- State changes in model objects trigger notifications
- Views register as observers and update when notified
- This allows for loose coupling between components

### Factory Pattern

Factory patterns are used to create objects of specific types dynamically:

- `TObjectFactory`: Creates instances of objects based on type identifiers
- `TModeFactory`: Creates interaction modes for different operations
- Various specialized factories for crystallographic objects

### Command Pattern

User actions are encapsulated as commands that can be executed, undone, and redone:

- `IOlxAction`: Interface for executable actions
- `IUndoAction`: Interface for undoable actions
- `TUndoStack`: Manages the undo/redo history

## Data Flow

1. Crystallographic data is loaded from files (CIF, SHELX, etc.) using parsers in the XLIB layer
2. Data is processed and transformed into internal object representations
3. The GXLIB layer creates visual representations of the crystallographic objects
4. The GLIB layer handles the rendering of these visual representations
5. The application layer (OLEX) handles user interactions and updates the model accordingly
6. Changes to the model trigger updates to the visual representation

## Technologies Used

- C++ as the primary programming language
- OpenGL for 3D rendering
- Python integration for scripting and extensions
- wxWidgets for cross-platform GUI (Windows/Linux/macOS)

## Cross-Cutting Concerns

- **Logging**: Centralized logging system for debugging and diagnostics
- **Error Handling**: Exception-based error handling with appropriate recovery
- **Internationalization**: Support for multiple languages
- **Configuration**: Settings and preferences management
- **Threading**: Multi-threaded processing for performance-critical operations
