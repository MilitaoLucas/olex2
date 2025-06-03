# Graphics Library (GLIB)

## Overview

The Graphics Library (GLIB) provides the visualization capabilities for Olex2, implementing OpenGL-based rendering of 3D graphics. It handles scene management, materials, lighting, camera control, and primitive rendering, providing a foundation for the crystallographic visualization in Olex2.

## Key Features

- **OpenGL Integration**: Abstraction of OpenGL functionality
- **Scene Management**: 3D scene graph and object management
- **Materials and Textures**: Material properties and texture handling
- **Lighting and Shading**: Light sources and shading models
- **Camera Control**: View manipulation and perspective control
- **Primitive Rendering**: Efficient rendering of common 3D primitives
- **Text and Font Handling**: Text rendering in 3D space

## Core Components

### Scene Management

- `AGlScene`: Abstract base class for 3D scenes
- `TGlRenderer`: Renderer for 3D objects
- `AGDrawObject`: Base class for drawable objects
- `TGlGroup`: Group of drawable objects for hierarchical organization

### Materials and Appearance

- `TGlMaterial`: Material properties for 3D objects
- `TGlTexture`: Texture handling and mapping
- `TGlLightModel`: Lighting model configuration
- `TGraphicsStyle`: Style definitions for graphical elements
- `TGraphicsStyles`: Collection of graphics styles

### Primitive Objects

- `TGlPrimitive`: Base class for primitive 3D objects
- `GlSphereEx`: Optimized sphere rendering
- `OlxSphere`: Enhanced sphere with additional properties
- `GlTorus`: Torus primitive
- `T3DFrameCtrl`: 3D coordinate frame visualization

### Text and Fonts

- `TGlFont`: Font handling for 3D text
- `TXGlLabels`: Labels in 3D space
- `TGlTextBox`: Text box for 3D scene

### User Interaction

- `TGlMouse`: Mouse handling in 3D scenes
- `AGlMouseHandler`: Abstract base class for mouse interaction
- `TGlClipPlanes`: Clipping planes for view control

### Specialized Rendering

- `OrtDraw`: Orthographic drawing utilities
- `OrtDrawTex`: Texture-based orthographic drawing
- `PSWriter`: PostScript output generation
- `PovDraw`: POV-Ray scene export
- `WrlDraw`: VRML/X3D scene export

## Design Patterns

The GLIB implements several design patterns:

- **Composite Pattern**: For scene graph hierarchy (e.g., `TGlGroup`)
- **Strategy Pattern**: For rendering algorithms
- **Observer Pattern**: For scene change notifications
- **Factory Pattern**: For creating graphical objects
- **Decorator Pattern**: For adding visual properties to objects

## Usage Examples

### Creating a Basic Scene

```cpp
// Initialize a renderer
TGlRenderer renderer;

// Create a scene
AGlScene* scene = new AGlScene();
scene->SetRenderer(renderer);

// Add a primitive to the scene
TGlPrimitive* sphere = new TGlPrimitive(sgSphere);
sphere->SetPosition(vec3d(0, 0, 0));
sphere->SetRadius(1.0);

// Set material properties
TGlMaterial material;
material.SetDiffuse(0.8, 0.2, 0.2, 1.0);
material.SetSpecular(1.0, 1.0, 1.0, 1.0);
material.SetShininess(50);
sphere->SetMaterial(material);

// Add to scene
scene->AddObject(*sphere);

// Render the scene
scene->Render();
```

### Working with Groups

```cpp
// Create a group
TGlGroup* group = new TGlGroup();

// Add primitives to the group
for (int i = 0; i < 5; i++) {
    TGlPrimitive* sphere = new TGlPrimitive(sgSphere);
    sphere->SetPosition(vec3d(i*2.0, 0, 0));
    sphere->SetRadius(0.5);
    group->AddObject(*sphere);
}

// Transform the entire group
group->SetPosition(vec3d(0, 5, 0));
group->SetRotation(quaterniond(vec3d(0,1,0), M_PI/4));

// Add the group to the scene
scene->AddObject(*group);
```

### Creating Labels

```cpp
// Create a label
TXGlLabels* labels = new TXGlLabels();
labels->SetFont(scene->GetFont());
labels->SetFontSize(12);

// Add text at specific 3D position
labels->AddLabel("C1", vec3d(1, 2, 3), 0);

// Configure appearance
labels->SetColor(0, 0, 0, 1);  // Black text
labels->SetBackgroundColor(1, 1, 1, 0.5);  // Semi-transparent white background

// Add to scene
scene->AddObject(*labels);
```

## Camera Control

```cpp
// Get the camera
TGlCamera& camera = scene->GetCamera();

// Set camera position and target
camera.SetPosition(vec3d(0, 0, 10));
camera.SetTarget(vec3d(0, 0, 0));

// Set perspective parameters
camera.SetFOV(45);
camera.SetNearClip(0.1);
camera.SetFarClip(100);

// Orbit the camera
camera.Orbit(vec3d(0,1,0), M_PI/6);
```

## File Organization

The GLIB is primarily located in the `glib` directory and includes:

- Core rendering classes
- Primitive object implementations
- Material and texture handling
- Camera and view control
- Scene graph management

## Dependencies

The GLIB depends on:

- SDL for fundamental data structures and utilities
- OpenGL for 3D graphics rendering
- Platform-specific windowing systems (indirectly)

## Extension Points

The GLIB is designed to be extendable through:

- Implementation of new primitive types
- Creation of specialized rendering techniques
- Addition of new export formats
- Extension of material and texture capabilities
