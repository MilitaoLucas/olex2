# Extending Olex2

This guide provides detailed information for developers who want to extend Olex2's functionality through plugins, scripts, or direct code modifications.

## Extension Methods

Olex2 can be extended in several ways:

1. **Python Scripts**: The easiest way to add functionality
2. **Plugins**: Dynamically loaded libraries that add new features
3. **Direct Code Modification**: Changing the core codebase
4. **Custom Commands**: Adding new commands to the command system
5. **User Interface Extensions**: Adding new UI elements

## Python Extensions

### Python API

Olex2 embeds Python and exposes its functionality through the `olex` module. This provides a powerful way to extend Olex2 without modifying its core code.

```python
import olex
from olexpy import utils

# Access the current structure
asymm_unit = olex.xf.au

# Get all atoms
atoms = asymm_unit.atoms()

# Calculate something
total_weight = sum(atom.mass() for atom in atoms)

# Display result
olex.m("Total molecular weight: {:.2f}".format(total_weight))
```

### Python Hooks

Key points where Python code can be triggered:

- **Startup**: Scripts in the `startup` directory run when Olex2 starts
- **File Loading**: Hooks triggered when a file is loaded
- **Command Execution**: Custom Python commands
- **Menu Actions**: Python functions triggered from menus
- **Periodic Events**: Scripts that run periodically

### Creating Python Extensions

1. **Create a Python Module**:
   Create a `.py` file in the `scripts` directory.

2. **Register Commands**:
   ```python
   import olex

   def my_function(arg1, arg2=None):
       # Function implementation
       return result

   # Register the command
   olex.RegisterFunction(my_function)
   ```

3. **Create User Interface**:
   ```python
   import olex
   from olexpy import gui

   # Create a panel
   panel = gui.Panel("My Extension")
   
   # Add controls
   panel.add_button("Execute", lambda: my_function())
   panel.add_textbox("Parameter", "default value")
   
   # Show the panel
   panel.show()
   ```

## Plugin Development

### Plugin Architecture

Olex2 plugins are dynamic libraries (DLLs/shared objects) that implement specific interfaces and are loaded at runtime.

```
┌───────────────────┐
│     Olex2 Core    │
│                   │
│  ┌─────────────┐  │
│  │ Plugin API  │◄─┼────┐
│  └─────────────┘  │    │
└───────────────────┘    │
                         │
┌───────────────────┐    │
│ Extension Plugin  │    │
│                   │    │
│  ┌─────────────┐  │    │
│  │Plugin Impl. ├──┼────┘
│  └─────────────┘  │
└───────────────────┘
```

### Plugin Interfaces

Olex2 defines several interfaces that plugins can implement:

- `IDllOlex2`: Main plugin interface
- `IOlex2Processor`: For processing crystallographic data
- `IOlex2Runnable`: For runnable extensions

### Creating a Plugin

1. **Create a New Library Project**:
   Set up a dynamic library project using your preferred build system.

2. **Include Olex2 Headers**:
   ```cpp
   #include "olex2app_imp.h"
   ```

3. **Implement the Plugin Interface**:
   ```cpp
   class MyPlugin : public olex2::IDllOlex2 {
   public:
       // Initialize the plugin
       virtual bool Init(olex2::IOlex2Processor* oxp) override {
           processor = oxp;
           // Register functionality
           return true;
       }
       
       // Clean up when unloaded
       virtual void Destroy() override {
           // Clean up resources
       }
       
       // Plugin identification
       virtual const char* GetName() const override {
           return "MyPlugin";
       }
       
       virtual const char* GetVersion() const override {
           return "1.0.0";
       }
   
   private:
       olex2::IOlex2Processor* processor;
   };
   
   // Export factory function
   extern "C" EXPORT_SYMBOL olex2::IDllOlex2* GetOlex2Object() {
       return new MyPlugin();
   }
   ```

4. **Register Functionality**:
   ```cpp
   bool MyPlugin::Init(olex2::IOlex2Processor* oxp) {
       processor = oxp;
       
       // Register a command
       processor->RegisterFunction("my_command", [this](const TStrObjList& params) {
           // Command implementation
           return new TMacroData();
       });
       
       // Register a menu item
       processor->RegisterMenu("Tools/My Plugin/Do Something", "my_command");
       
       return true;
   }
   ```

5. **Build and Deploy**:
   Compile your plugin as a dynamic library and place it in Olex2's `plugins` directory.

## Custom Commands

### Command System

Olex2's command system allows adding new commands without creating full plugins.

### Adding Commands in C++

```cpp
// Define a command handler
class MyCommandAction : public IOlxAction {
public:
    // Execute the command
    virtual bool Execute(const TStrObjList& params) {
        // Command implementation
        return true;
    }
    
    // Command help
    virtual olxstr GetDescription() const {
        return "My custom command";
    }
};

// Register the command
TBasicApp::GetInstance().ProcessManager.AddCommand(
    "my_command", new MyCommandAction());
```

### Command Parameters

Commands can accept parameters:

```cpp
bool Execute(const TStrObjList& params) {
    if (params.Count() < 1)
        return false;
        
    olxstr param1 = params[0];
    double param2 = (params.Count() > 1) ? params[1].ToDouble() : 1.0;
    
    // Use parameters
    return true;
}
```

## UI Extensions

### Custom Dialogs

Creating custom dialogs:

```cpp
class MyDialog : public wxDialog {
public:
    MyDialog(wxWindow* parent) : 
        wxDialog(parent, wxID_ANY, "My Dialog", 
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    {
        // Create controls
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        
        wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY);
        sizer->Add(textCtrl, 1, wxEXPAND | wxALL, 10);
        
        wxButton* button = new wxButton(this, wxID_OK, "OK");
        sizer->Add(button, 0, wxALIGN_RIGHT | wxALL, 10);
        
        SetSizer(sizer);
        Layout();
        Fit();
    }
};

// Show the dialog
MyDialog dialog(TMainForm::GetInstance());
if (dialog.ShowModal() == wxID_OK) {
    // Process result
}
```

### Custom Visualization

Adding custom visualization elements:

```cpp
// Create a custom drawable object
class MyDrawObject : public AGDrawObject {
public:
    MyDrawObject() : AGDrawObject("MyObject") {
        // Initialize properties
    }
    
    virtual void Render(TGlRenderer& renderer) override {
        // Custom rendering code
        renderer.PushMatrix();
        // Draw using OpenGL or GLIB primitives
        renderer.PopMatrix();
    }
    
    virtual bool IsVisible() const override {
        return true;
    }
};

// Add to the scene
TGlGroup& group = TXApp::GetInstance().GetRenderer().FindGroup("MyGroup");
MyDrawObject* obj = new MyDrawObject();
group.AddObject(*obj);
```

## Direct Code Modification

### Key Extension Points

If modifying the core code, these are good places to extend:

1. **XApp Class**: Central application class (`TXApp` in `xapp.h`)
2. **Command Processor**: Command handling (`TBasicApp::ProcessManager`)
3. **GUI Integration**: Main form (`TMainForm` in `mainform.h`)
4. **Visualization Pipeline**: Scene management (`TGlRenderer` in `glrender.h`)
5. **File Handlers**: File format support (`TFileHandlerManager`)

### Adding New File Formats

To add support for a new file format:

```cpp
// Create a file handler
class MyFormatHandler : public IOlexFileHandler {
public:
    // Check if this handler can process the file
    virtual bool CheckFile(const olxstr& file) override {
        return file.EndsWith(".myformat");
    }
    
    // Process the file
    virtual bool Process(const olxstr& file) override {
        // Parse the file
        // Create atoms, bonds, etc.
        return true;
    }
};

// Register the handler
TFileHandlerManager::GetInstance().RegisterHandler(
    new MyFormatHandler());
```

## Best Practices

### Integration Guidelines

1. **Use Existing Abstractions**: Work with existing classes rather than creating parallel hierarchies
2. **Follow Patterns**: Use the same design patterns as the core code
3. **Error Handling**: Use exceptions consistently with the core code
4. **Threading**: Respect the threading model; UI updates on the main thread
5. **Memory Management**: Use Olex2's smart pointers and RAII patterns

### Performance Considerations

1. **Heavy Computation**: Move intensive calculations to background threads
2. **Efficient Rendering**: Minimize OpenGL state changes and draw calls
3. **Memory Usage**: Be careful with large data structures
4. **Caching**: Cache results of expensive operations
5. **Progress Feedback**: Provide progress feedback for long operations

### Testing Extensions

1. **Isolated Testing**: Test components in isolation
2. **Integration Testing**: Test integration with Olex2
3. **Edge Cases**: Test with unusual or large structures
4. **Platform Testing**: Test on all target platforms
5. **Clean State**: Test starting from a clean state

## Debugging Extensions

### Tools and Techniques

1. **Logging**: Use `TLog::GetInstance().WriteLine()` for debugging
2. **Debug Builds**: Use debug builds with additional checks
3. **Visual Debugging**: Add visual elements to debug spatial issues
4. **Command Output**: Output debug information to the command window
5. **Step-by-Step**: Add checkpoints to complex algorithms

### Common Issues

1. **Memory Leaks**: Not properly managing object lifecycles
2. **Thread Synchronization**: Thread safety issues
3. **OpenGL State**: Not properly restoring OpenGL state
4. **Performance**: Inefficient algorithms or rendering
5. **UI Responsiveness**: Blocking the main thread
