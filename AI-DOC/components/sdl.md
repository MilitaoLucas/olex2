# Software Development Library (SDL)

## Overview

The Software Development Library (SDL) serves as the foundation layer for Olex2, providing essential utilities, data structures, and algorithms used throughout the application. It's designed to be independent of crystallography-specific code, making it potentially reusable in other scientific software projects.

## Key Features

- **Data Structures**: Implementation of fundamental and specialized data structures
- **Memory Management**: Smart pointers and memory allocation utilities
- **File Handling**: File I/O, serialization, and path management
- **Exception Handling**: Comprehensive exception hierarchy
- **Multithreading**: Thread management and synchronization primitives
- **Mathematical Utilities**: Vector, matrix operations, and numerical algorithms
- **Parsing**: Lexical analysis and parsing frameworks

## Core Components

### Data Structures

#### Collections
- `TArrayList<T>`: Templated dynamic array implementation
- `TPtrList<T>`: List for managing pointers to objects
- `TTStrList`: Specialized string list with additional functionality
- `TTypeList<T>`: Type-specific list with extended operations
- `olxdict<K,V>`: Dictionary (hash map) implementation
- `BTree<K,V>`: Balanced tree implementation
- `TLinkedList<T>`: Doubly-linked list implementation

#### Smart Pointers
- `olx_ptr<T>`: Basic smart pointer with reference counting
- `olx_virtual_ptr<T>`: Smart pointer for polymorphic types
- `olx_object_ptr<T>`: Smart pointer specifically for IOlxObject derived classes
- `olx_perishable_ptr<T>`: Smart pointer for APerishable derived classes

### File System

- `TEFile`: Enhanced file handling with cross-platform path management
- `TFileSystem`: Abstract interface for file system operations
- `TOSFileSystem`: Native file system implementation
- `THttpFileSystem`: HTTP-based file system for remote resources
- `TZipFS`: File system for ZIP archives
- `TUpdateFS`: File system for handling updates

### Exception Handling

- `TExceptionBase`: Base class for all exceptions
- `TFunctionFailedException`: Indicates a function execution failure
- `TInvalidArgumentException`: Signals invalid function arguments
- `TIOException`: File I/O related exceptions
- `TOutOfMemoryException`: Memory allocation failures
- `TMathException`: Mathematical operation errors

### Multithreading

- `AOlxThread`: Abstract base class for thread implementation
- `TThreadPool`: Thread pool for task execution
- `olx_critical_section`: RAII wrapper for critical sections
- `AProcess`: Base class for process execution
- `ProcessManager`: Management of external processes

### Mathematics

- `TVector<T>`: Vector template with mathematical operations
- `TMatrix<T>`: Matrix template with mathematical operations
- `TEllipsoid`: Ellipsoid representation and calculations
- `TPolynom`: Polynomial representation and evaluation
- `TEValue`: Floating-point value with error (standard deviation)

### Parsing

- `TExpressionParser`: Expression parsing and evaluation
- `TIEvaluable`: Interface for evaluable expressions
- `IEvaluator`: Interface for expression evaluators
- `IEvaluatorFactory`: Factory for creating evaluators

## Design Patterns

The SDL implements several design patterns:

- **Factory Pattern**: For creating objects (e.g., `TObjectFactory`)
- **Observer Pattern**: For event notification (e.g., `AEventsDispatcher`)
- **Strategy Pattern**: For interchangeable algorithms
- **Singleton Pattern**: For unique instances (e.g., `TLog`)
- **RAII Pattern**: For resource management (e.g., `olx_scope_cs`)

## Usage Examples

### Smart Pointer Usage

```cpp
// Creating a smart pointer
olx_ptr<TDataItem> item = new TDataItem();

// Smart pointer automatically manages memory
// No need to delete the object

// Checking if pointer is valid
if (item.ok()) {
    // Use the pointer
}
```

### Collection Usage

```cpp
// Creating a string list
TTStrList strings;
strings.Add("First item");
strings.Add("Second item");

// Iterating through the list
for (size_t i = 0; i < strings.Count(); i++) {
    const olxstr& str = strings[i];
    // Process string
}
```

### Exception Handling

```cpp
try {
    // Operation that might fail
    TEFile file("filename.txt", "rb");
    // Process file
}
catch (const TIOException& e) {
    // Handle file I/O exception
    TLog::GetInstance().WriteLine(e.GetException());
}
```

## File Organization

The SDL is primarily located in the `sdl` directory and includes:

- Header files defining interfaces and templates
- Implementation files with concrete functionality
- Test files for unit testing

## Dependencies

The SDL has minimal external dependencies:

- Standard C++ Library
- Platform-specific APIs for OS interaction
- OpenSSL for cryptographic functions (optional)

## Extension Points

The SDL is designed to be extendable through:

- Inheritance from base classes
- Implementation of interfaces
- Template specialization
- Factory registration
