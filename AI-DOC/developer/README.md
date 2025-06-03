# Olex2 Developer Guide

## Introduction

This guide is intended for developers who want to understand, maintain, or extend the Olex2 codebase. It provides practical information about the development environment, coding standards, build process, and extension points.

## Getting Started

### Development Environment Setup

To set up a development environment for Olex2:

1. **Clone the Repository**
   ```bash
   git clone https://github.com/your-organization/olex2.git
   cd olex2
   ```

2. **Dependencies**
   Ensure you have the following dependencies installed:
   - C++ compiler (GCC 7+, MSVC 2017+, or Clang)
   - CMake 3.10+
   - Python 3.8+
   - OpenGL development libraries
   - wxWidgets 3.0+

3. **Build Configuration**
   For Linux/macOS:
   ```bash
   mkdir build && cd build
   cmake .. -GNinja \
          -DPYTHON_LIBRARY=/path/to/python/lib/libpython3.8.so \
          -DPYTHON_INCLUDE_DIR=/path/to/python/include \
          -DPYTHON_ROOT_DIR=/path/to/python \
          -DPython3_FIND_STRATEGY="LOCATION"
   ```

   For Windows:
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64 ^
          -DPYTHON_LIBRARY=C:/path/to/python/libs/python38.lib ^
          -DPYTHON_INCLUDE_DIR=C:/path/to/python/include ^
          -DPYTHON_ROOT_DIR=C:/path/to/python
   ```

4. **Building**
   ```bash
   # Using Ninja
   ninja

   # Using Make
   make

   # Using Visual Studio
   cmake --build . --config Release
   ```

### Project Structure

The source code is organized into several key directories:

- `sdl/`: Software Development Library - core utilities
- `xlib/`: Crystallography Library - crystallographic calculations
- `glib/`: Graphics Library - OpenGL rendering
- `gxlib/`: Graphics Extensions - crystallographic visualization
- `olex/`: Application Layer - UI and application code
- `unirun/`: Universal runner for crystallographic operations
- `scripts/`: Various utility scripts
- `tests/`: Unit and integration tests
- `examples/`: Example code and use cases

### Build System

Olex2 uses CMake as its primary build system:

- `CMakeLists.txt` files define the build process
- `cmake/` directory contains custom CMake modules
- The build system supports different configurations (Debug, Release, etc.)
- Cross-platform builds are supported for Windows, Linux, and macOS

## Coding Standards

### C++ Style Guide

Olex2 follows these coding conventions:

- **Naming Conventions**
  - Classes: `TClassName` or `AClassName` (A for abstract classes)
  - Interfaces: `IInterfaceName`
  - Methods: `MethodName()` (PascalCase)
  - Variables: `variable_name` (snake_case) or `memberVariable` (camelCase)
  - Constants: `CONSTANT_NAME` (uppercase with underscores)

- **Code Organization**
  - One class per file (generally)
  - Header files use include guards or `#pragma once`
  - Implementation in .cpp files
  - Related classes grouped in directories

- **Documentation**
  - Doxygen-style comments for public API
  - Brief description for all classes and methods
  - Parameter descriptions for non-obvious parameters
  - Implementation notes for complex algorithms

### Memory Management

Olex2 uses several memory management approaches:

- Custom smart pointers (`olx_ptr<T>`, `olx_object_ptr<T>`)
- RAII (Resource Acquisition Is Initialization) pattern
- Clear ownership semantics for object lifecycles
- Memory pools for frequently allocated objects

### Error Handling

Error handling strategy:

- Exceptions for error conditions that cannot be handled locally
- Return values for expected failure conditions
- Assertions for programming errors
- Logging for diagnostic information

## Architecture and Design

### Design Patterns

Olex2 uses several design patterns:

- **Factory Pattern**: For creating objects (e.g., `TObjectFactory`)
- **Observer Pattern**: For event notification (e.g., `AEventsDispatcher`)
- **Command Pattern**: For undoable operations
- **Strategy Pattern**: For interchangeable algorithms
- **Composite Pattern**: For object hierarchies (e.g., scene graph)

### Key Abstractions

Important abstractions to understand:

- **Objects**: `IOlxObject` - base for identifiable objects
- **References**: `AReferencible` - base for referable objects
- **Perishables**: `APerishable` - objects with cleanup requirements
- **Events**: `AEventsDispatcher` - for event notification
- **Actions**: `IOlxAction` - encapsulated operations
- **Data Items**: `TDataItem` - for structured data storage

### Threading Model

Understanding the threading model:

- Main thread handles UI and user interaction
- Background threads for computation-intensive tasks
- Thread pool for parallel operations
- Synchronization through critical sections and mutexes
- Thread-safe communication through message queues

## Extension Points

### Plugin System

Olex2 can be extended through plugins:

- **Libraries**: Dynamic libraries loaded at runtime
- **Registration**: Plugins register their functionality with central registries
- **Interfaces**: Plugins implement standard interfaces for integration

### Python Integration

Python integration enables scripting and extensions:

- Python interpreter embedded in Olex2
- Python API exposing Olex2 functionality
- Python scripts can be executed from within Olex2
- Custom Python modules can extend functionality

### Custom Commands

Adding custom commands:

- Register new commands with the command processor
- Implement command handlers
- Provide help and documentation
- Add to appropriate menus or toolbars

### Custom Visualization

Extending visualization capabilities:

- Create new drawable objects inheriting from `AGDrawObject`
- Implement rendering methods
- Register with the scene management
- Add user interface for controlling properties

## Testing

### Unit Testing

Olex2 uses a custom unit testing framework:

- Test cases defined in `tests/` directory
- Tests can be run individually or as suites
- Test coverage is tracked

### Integration Testing

Integration tests verify component interactions:

- End-to-end tests for major workflows
- Component integration tests
- Test data in the `tests/data/` directory

### Test Utilities

Utilities for testing:

- Mock objects for dependencies
- Test fixtures for common setups
- Assertion macros for verification
- Tools for measuring performance

## Debugging

### Logging

Olex2 has a comprehensive logging system:

- Multiple log levels (debug, info, warning, error)
- Log outputs to console and file
- Component-specific logging
- Performance logging for optimization

### Debugging Tools

Tools for debugging:

- Built-in debug visualization
- Memory usage tracking
- Performance profiling
- Debug builds with additional checks

## Contributing

### Pull Request Process

Steps for contributing:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request with a description of changes

### Code Review

Code review expectations:

- All code changes are reviewed before merging
- Automated checks for coding standards
- Manual review for design and implementation
- Performance considerations for critical paths

## Documentation

### API Documentation

Maintaining API documentation:

- Use Doxygen comments for all public interfaces
- Update documentation when changing interfaces
- Generate Doxygen documentation to verify changes
- Keep examples up-to-date

### Internal Documentation

Internal documentation practices:

- Comment complex algorithms
- Document design decisions
- Maintain architecture documentation
- Update interaction diagrams when changing component relationships

## Build and Deployment

### Continuous Integration

CI/CD workflow:

- Automated builds on all supported platforms
- Unit and integration tests run automatically
- Performance benchmarks for critical paths
- Code quality metrics tracked

### Release Process

Steps for releasing new versions:

1. Version numbering (Major.Minor.Patch)
2. Change log maintenance
3. Release branch creation
4. Testing and validation
5. Binary building and packaging
6. Documentation updates
7. Release notes preparation
8. Distribution

## Troubleshooting

### Common Issues

Solutions for common development issues:

- Build failures and their common causes
- Runtime crashes and debugging approaches
- Performance bottlenecks and profiling
- Platform-specific issues

### Support Resources

Where to get help:

- Developer mailing lists
- Issue tracker
- Developer documentation
- Community forums
