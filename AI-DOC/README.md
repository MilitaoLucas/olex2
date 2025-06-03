# Olex2 Documentation
Build by AI, it is temporary for now. DON'T TRUST IT NOT OFFICIAL
## Overview

Olex2 is a comprehensive crystallography software package designed for crystal structure determination, refinement, and analysis. It provides crystallographers with a powerful set of tools for working with crystallographic data, from initial data processing to final structure visualization and publication.

This documentation provides a detailed explanation of Olex2's architecture, components, and functionality, aimed at developers who want to understand, maintain, or extend the software.

## Key Features

- Interactive 3D visualization of crystal structures
- Crystallographic calculations and refinement
- CIF (Crystallographic Information File) handling
- Symmetry operations and space group analysis
- Molecular geometry analysis
- Structure validation and reporting
- Publication-quality graphics generation

## Project Structure

Olex2 is organized into several main components:

1. **Core Libraries**
   - `sdl`: Software Development Library - fundamental data structures and utilities
   - `xlib`: Crystallography Library - crystallographic calculations and data structures
   - `glib`: Graphics Library - OpenGL-based rendering and visualization
   - `gxlib`: Graphics Extensions Library - specialized crystallographic visualization

2. **Application Layer**
   - `olex`: Main application code including UI and application-specific functionality

3. **Documentation**
   - `doxygen`: Auto-generated API documentation
   - `AI-DOC`: This comprehensive documentation

## Documentation Contents

- [Architecture Overview](architecture.md) - High-level architecture of Olex2
- [Core Components](components/README.md) - Detailed description of main components
- [Developer Guide](developer/README.md) - Information for developers
- [Class Reference](classes/README.md) - Key class documentation

## License

See the `LICENCE.txt` file in the root directory for licensing information.
