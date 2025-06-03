# Key Classes in Olex2

This document provides an overview of the most important classes in Olex2, organized by component. Understanding these classes is essential for developers who want to extend or modify the software.

## Software Development Library (SDL)

### Core Utilities

#### `TBasicApp`
The base application class that provides core functionality like command processing, event handling, and application lifecycle management.

```cpp
class TBasicApp {
public:
    // Singleton access
    static TBasicApp& GetInstance();
    
    // Event handling
    virtual bool ProcessEvent(int eventId, int eventData);
    
    // Command processing
    TActionQueue& GetActionQueue();
    
    // Application lifecycle
    virtual bool Init();
    virtual void Exit();
};
```

#### `TLog`
Centralized logging system for the application.

```cpp
class TLog {
public:
    static TLog& GetInstance();
    
    void WriteLine(const olxstr& line, short level=0);
    void LogAttribute(const olxstr& name, const olxstr& value);
    
    void Enter(const olxstr& scope);
    void Exit();
};
```

### Data Structures

#### `TArrayList<T>`
Dynamic array implementation with extensive functionality.

```cpp
template <class T>
class TArrayList {
public:
    size_t Add(const T& item);
    void Insert(size_t index, const T& item);
    void Delete(size_t index);
    void Clear();
    
    size_t Count() const;
    T& operator [](size_t index);
    const T& operator [](size_t index) const;
};
```

#### `olxdict<K,V>`
Dictionary (hash map) implementation.

```cpp
template <typename KeyT, class ValT>
class olxdict {
public:
    void Add(const KeyT& key, const ValT& value);
    void Remove(const KeyT& key);
    bool HasKey(const KeyT& key) const;
    
    ValT& operator [](const KeyT& key);
    const ValT& operator [](const KeyT& key) const;
};
```

### File System

#### `TEFile`
Enhanced file handling with path management.

```cpp
class TEFile {
public:
    TEFile(const olxstr& fileName, const olxstr& mode);
    
    static bool Exists(const olxstr& fileName);
    static bool Delete(const olxstr& fileName);
    static bool Rename(const olxstr& oldName, const olxstr& newName);
    
    bool IsOpen() const;
    void Close();
    
    size_t Read(void* buffer, size_t size, size_t count);
    size_t Write(const void* buffer, size_t size, size_t count);
};
```

### Memory Management

#### `olx_ptr<T>`
Smart pointer with reference counting.

```cpp
template <typename T>
class olx_ptr {
public:
    olx_ptr(T* ptr = NULL);
    olx_ptr(const olx_ptr<T>& o);
    ~olx_ptr();
    
    T* operator ->() const;
    T& operator *() const;
    bool ok() const;  // Check if pointer is valid
};
```

## Crystallography Library (XLIB)

### Crystallographic Objects

#### `TCAtom`
Representation of a crystallographic atom.

```cpp
class TCAtom {
public:
    // Basic properties
    const olxstr& GetLabel() const;
    void SetLabel(const olxstr& label);
    
    const olxstr& GetType() const;
    void SetType(const olxstr& type);
    
    // Coordinates
    vec3d& ccrd();  // Cartesian coordinates
    const vec3d& ccrd() const;
    
    vec3d& crd();   // Fractional coordinates
    const vec3d& crd() const;
    
    // Properties
    double GetOccu() const;
    void SetOccu(double occu);
    
    int GetPart() const;
    void SetPart(int part);
};
```

#### `TUnitCell`
Unit cell representation with transformation methods.

```cpp
class TUnitCell {
public:
    void SetCell(double a, double b, double c, 
                double alpha, double beta, double gamma);
    
    double CalcVolume() const;
    double CalcDensity() const;
    
    vec3d ToCartesian(const vec3d& fc) const;
    vec3d ToFractional(const vec3d& cc) const;
    
    void SetSpaceGroup(const TSpaceGroup& sg);
    const TSpaceGroup& GetSpaceGroup() const;
};
```

#### `TSpaceGroup`
Space group representation with symmetry operations.

```cpp
class TSpaceGroup {
public:
    olxstr GetName() const;
    olxstr GetHallSymbol() const;
    
    size_t GetEuivPositionCount() const;
    TSymmElement& GetEquivPosition(size_t i);
    
    bool IsCentrosymmetric() const;
    int GetLatticeType() const;
    
    static TSpaceGroup& GetP1();  // P1 space group
};
```

### File Format Handling

#### `TIns`
SHELX instruction file parser and generator.

```cpp
class TIns {
public:
    bool LoadFromFile(const olxstr& fileName);
    bool SaveToFile(const olxstr& fileName) const;
    
    TAsymmUnit& GetAsymmUnit();
    const TAsymmUnit& GetAsymmUnit() const;
    
    olxstr GetTitle() const;
    void SetTitle(const olxstr& title);
};
```

#### `TCif`
CIF file parser and generator.

```cpp
class TCif {
public:
    bool LoadFromFile(const olxstr& fileName);
    bool SaveToFile(const olxstr& fileName) const;
    
    // Access to CIF blocks
    size_t BlockCount() const;
    TCifBlock& GetBlock(size_t i);
    TCifBlock& GetBlock(const olxstr& name);
};
```

## Graphics Library (GLIB)

### Scene Management

#### `AGlScene`
Abstract base class for 3D scenes.

```cpp
class AGlScene {
public:
    virtual void Render() = 0;
    virtual void Update() = 0;
    
    void AddObject(AGDrawObject& obj);
    void RemoveObject(AGDrawObject& obj);
    
    TGlRenderer& GetRenderer();
    void SetRenderer(TGlRenderer& renderer);
    
    TGlCamera& GetCamera();
};
```

#### `TGlRenderer`
Renderer for 3D objects.

```cpp
class TGlRenderer {
public:
    void BeginRendering();
    void EndRendering();
    
    void RenderObject(AGDrawObject& obj);
    
    void PushMatrix();
    void PopMatrix();
    void Translate(const vec3d& t);
    void Rotate(const quaterniond& q);
    void Scale(const vec3d& s);
};
```

### Primitive Objects

#### `AGDrawObject`
Base class for drawable objects.

```cpp
class AGDrawObject {
public:
    AGDrawObject(const olxstr& id);
    
    virtual void Render(TGlRenderer& renderer) = 0;
    virtual bool IsVisible() const = 0;
    
    const olxstr& GetId() const;
    
    void SetPosition(const vec3d& pos);
    const vec3d& GetPosition() const;
    
    void SetRotation(const quaterniond& rot);
    const quaterniond& GetRotation() const;
};
```

#### `TGlPrimitive`
Base class for primitive 3D objects.

```cpp
class TGlPrimitive : public AGDrawObject {
public:
    enum Type {
        sgNone,
        sgSphere,
        sgCylinder,
        sgCone,
        sgQuad,
        sgTriangle
    };
    
    TGlPrimitive(Type type);
    
    void SetColor(float r, float g, float b, float a = 1.0f);
    void SetMaterial(const TGlMaterial& material);
    
    void SetRadius(float radius);  // For spheres
    void SetHeight(float height);  // For cylinders/cones
};
```

## Graphics Extensions Library (GXLIB)

### Crystallographic Visualization

#### `TXAtom`
Visual representation of atoms.

```cpp
class TXAtom : public TGlPrimitive {
public:
    TXAtom(TCAtom& atom);
    
    TCAtom& GetCAtom();
    const TCAtom& GetCAtom() const;
    
    void SetLabel(const olxstr& label);
    const olxstr& GetLabel() const;
    
    void SetStyle(AtomRenderingStyle style);
    AtomRenderingStyle GetStyle() const;
    
    void UpdatePosition();  // Update from atom coordinates
};
```

#### `TXBond`
Visual representation of bonds.

```cpp
class TXBond : public TGlPrimitive {
public:
    TXBond(TXAtom& a1, TXAtom& a2);
    
    TXAtom& GetA();
    TXAtom& GetB();
    
    void SetStyle(BondStyle style);
    BondStyle GetStyle() const;
    
    void UpdatePosition();  // Update from atom positions
};
```

#### `TDUnitCell`
Visual representation of the unit cell.

```cpp
class TDUnitCell : public AGDrawObject {
public:
    void SetCell(double a, double b, double c, 
                double alpha, double beta, double gamma);
    
    void SetLineWidth(float width);
    void SetColor(float r, float g, float b, float a = 1.0f);
    
    void SetAxesVisible(bool visible);
    bool GetAxesVisible() const;
};
```

## Application Layer (OLEX)

### Application

#### `TXApp`
Main application class for Olex2.

```cpp
class TXApp : public TGXApp {
public:
    static TXApp& GetInstance();
    
    virtual bool Init();
    virtual void Exit();
    
    bool LoadFile(const olxstr& fileName);
    bool SaveFile(const olxstr& fileName);
    
    TUnitCell& GetUnitCell();
    TAsymmUnit& GetAsymmUnit();
    
    TGlRenderer& GetRenderer();
    AGlScene& GetScene();
};
```

#### `TMainForm`
Main application window and UI management.

```cpp
class TMainForm : public wxFrame {
public:
    static TMainForm& GetInstance();
    
    TGlCanvas& GetGlCanvas();
    
    void UpdateUI();
    void ShowMessage(const olxstr& message);
    
    void AddTool(const olxstr& id, const olxstr& label, 
                const olxstr& tooltip, const olxstr& bitmap);
    
    void AddMenu(const olxstr& path, const olxstr& command);
};
```

### Dialog Boxes

#### `TdlgMsgBox`
Message box dialog.

```cpp
class TdlgMsgBox : public wxDialog {
public:
    enum Type {
        mtInfo,
        mtWarning,
        mtError,
        mtQuestion
    };
    
    static int ShowMessage(const olxstr& message, const olxstr& caption, 
                         Type type = mtInfo);
    
    static bool AskYesNo(const olxstr& message, const olxstr& caption);
};
```

## Object Relationships

Understanding the relationships between these key classes is crucial:

### Hierarchy

```
TBasicApp
└── TXApp
    └── TMainForm
        └── TGlCanvas
            └── AGlScene
                └── AGDrawObject
                    ├── TGlPrimitive
                    │   ├── TXAtom
                    │   └── TXBond
                    └── TDUnitCell
```

### Composition

- `TXApp` contains `TAsymmUnit` and `TUnitCell`
- `TAsymmUnit` contains multiple `TCAtom` objects
- `AGlScene` contains multiple `AGDrawObject` objects
- `TXAtom` references a `TCAtom`
- `TXBond` references two `TXAtom` objects

## Memory Management

Understanding object ownership is important:

- Smart pointers (`olx_ptr<T>`) are used for many objects
- Visual objects (`AGDrawObject` derivatives) are typically owned by scenes
- Crystallographic objects (`TCAtom`, etc.) are typically owned by containers
- The application is responsible for synchronizing visual and crystallographic objects

## Threading Considerations

- UI operations must be performed on the main thread
- Long-running crystallographic operations should be performed on background threads
- Thread synchronization is needed when accessing shared data

## Common Patterns

- **Factory Pattern**: `TObjectFactory` creates objects based on type
- **Observer Pattern**: Objects notify observers of changes
- **Command Pattern**: Commands encapsulate operations
- **Visitor Pattern**: For traversing object hierarchies
