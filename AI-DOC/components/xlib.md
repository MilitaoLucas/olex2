# Crystallography Library (XLIB)

## Overview

The Crystallography Library (XLIB) is the core scientific component of Olex2, implementing domain-specific crystallographic functionality. It provides data structures and algorithms for representing and manipulating crystallographic concepts such as atoms, bonds, unit cells, space groups, and symmetry operations.

## Key Features

- **Crystallographic Data Structures**: Representation of atoms, bonds, unit cells, etc.
- **Space Group Handling**: Space group symmetry operations and transformations
- **Crystallographic File Formats**: Parsing and generation of CIF, SHELX, and other formats
- **Refinement Algorithms**: Structure refinement and validation
- **Symmetry Operations**: Implementation of crystallographic symmetry
- **Molecular Geometry Analysis**: Bond lengths, angles, and other geometric calculations
- **Constraints and Restraints**: Application of crystallographic constraints and restraints

## Core Components

### Crystallographic Objects

#### Atoms and Bonds
- `TCAtom`: Representation of a crystallographic atom with position, type, and properties
- `TSAtom`: SHELX-specific atom representation
- `TXAtom`: Extended atom representation with visualization properties
- `TSBond`: Bond between atoms with properties
- `TXBond`: Extended bond representation with visualization properties

#### Unit Cell and Lattice
- `TUnitCell`: Unit cell representation with transformation methods
- `TLattice`: Crystal lattice with periodicity and symmetry
- `TAsymmUnit`: Asymmetric unit representation

#### Space Groups and Symmetry
- `TSpaceGroup`: Space group representation with symmetry operations
- `TSymmLib`: Library of symmetry operations and space groups
- `TSymmElement`: Representation of symmetry elements
- `TSymmMat`: Symmetry matrix for transformations
- `FastSG_*`: Optimized space group implementations

### File Format Handling

- `TIns`: SHELX instruction file parser and generator
- `TLst`: SHELX list file parser
- `TCif`: CIF file parser and generator
- `TCifDataManager`: Management of CIF data
- `TMol`: Molecular file format handling
- `TP4P`: P4P file format (diffractometer data)
- `TXYZ`: XYZ file format for molecular coordinates

### Refinement and Analysis

- `TRefinementModel`: Model for structure refinement
- `TReflection`: Reflection data representation
- `XLibMacros`: Crystallographic calculation macros
- `TAnalysis`: Structure analysis utilities
- `TSGTest`: Space group validation tests
- `TAfixGroups`: Handling of SHELX AFIX constraints
- `TSameGroup`: SAME restraint handling
- `TLst`: Refinement results analysis

### Molecular Structure

- `TNetwork`: Molecular connectivity network
- `TFragment`: Molecular fragment handling
- `TNetTraverser`: Network traversal algorithms
- `TAtomSorter`: Atom sorting algorithms
- `TLLTBondSort`: Bond sorting algorithms

## Design Patterns

The XLIB implements several design patterns:

- **Factory Pattern**: For creating crystallographic objects
- **Observer Pattern**: For structure change notifications
- **Visitor Pattern**: For traversing crystallographic structures
- **Strategy Pattern**: For different refinement strategies
- **Command Pattern**: For undoable crystallographic operations

## Usage Examples

### Atom and Unit Cell Manipulation

```cpp
// Creating a unit cell
TUnitCell unitCell;
unitCell.SetCell(10.0, 10.0, 10.0, 90.0, 90.0, 90.0);

// Adding an atom to the asymmetric unit
TAsymmUnit au;
TCAtom& atom = au.NewAtom();
atom.SetLabel("C1");
atom.SetType("C");
atom.ccrd() = vec3d(0.1, 0.2, 0.3);  // Fractional coordinates

// Generating symmetry equivalent atoms
TSpaceGroup sg = TSymmLib::GetInstance().FindGroupByName("P 21/c");
unitCell.SetSpaceGroup(sg);
TArrayList<TCAtom*> symmAtoms;
unitCell.GenerateSymmAtoms(atom, symmAtoms);
```

### CIF File Handling

```cpp
// Reading a CIF file
TCif cif;
cif.LoadFromFile("structure.cif");

// Accessing CIF data
TCifDataManager dm;
dm.AssignCif(cif);
olxstr formula = dm.GetFormula();

// Getting atom data
TCAtomPList atoms;
dm.GetAsymmUnit(atoms);
```

### Structure Analysis

```cpp
// Calculating molecular weight
TAsymmUnit au;
// ... populate asymmetric unit ...
double mw = au.GetMolWeight();

// Calculating density
TUnitCell uc;
// ... set unit cell parameters ...
double density = uc.CalcDensity();

// Analyzing molecular geometry
TSBondPList bonds;
// ... populate bond list ...
for (size_t i=0; i < bonds.Count(); i++) {
    TSBond& bond = *bonds[i];
    double length = bond.Length();
    // Process bond length
}
```

## File Organization

The XLIB is primarily located in the `xlib` directory and includes:

- Core crystallographic classes
- File format parsers and generators
- Refinement and analysis algorithms
- Space group and symmetry implementations

## Dependencies

The XLIB depends on:

- SDL for fundamental data structures and utilities
- External crystallographic databases and tables
- Mathematical libraries for numerical calculations

## Extension Points

The XLIB is designed to be extendable through:

- Implementation of new file format parsers
- Addition of new refinement algorithms
- Extension of analysis capabilities
- Implementation of new crystallographic constraints and restraints
