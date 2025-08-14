# 🎠 Night Carousel Viewer

A 3D carousel scene built with **C++** and **OpenGL**, featuring lighting, skybox, animation, and free-roam camera support. Developed using Visual Studio.

---

## 📦 Features

- Spinning carousel model with animated horses
- Night skybox environment
- Ground glow and lighting effects
- Free-roam and cinematic camera modes
- Adjustable rotation speed
- Built entirely in modern OpenGL (core profile)

---

## 🛠️ Requirements

Before building this project, you’ll need the following tools installed:

### ✅ Visual Studio 2022 (with C++ Desktop Workload)
Ensure you’ve installed:

- **Desktop development with C++**
- **CMake tools for Windows**
- **Windows 10/11 SDK**

---

### ✅ [vcpkg](https://github.com/microsoft/vcpkg)

Install and integrate with Visual Studio:

```
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### ✅ Install Assimp with vcpkg
This project uses Assimp to load the .fbx carousel model. Install it like this:
```
.\vcpkg install assimp:x64-windows
```

### ⚠️ Make sure you're using the correct architecture (usually x64) when building.

### 📁 Project Structure
Night_Carousel/
│
├── external/            # Local libraries (GLFW, GLAD, GLM)
├── assets/              # Textures, shaders, model files
├── src/                 # Source files (main.cpp, ModelLoader.cpp/hpp)
├── CMakeLists.txt
├── CMakeSettings.json
└── README.md

### ▶️ Running the Project
Open Visual Studio 2022

Select File > Open > CMake... and choose the Night_Carousel project folder

Make sure x64-Debug or x64-Release is selected in the top-left configuration dropdown

Press Ctrl + Shift + B to build

Press F5 to run

### ⚠️ Asset Path Note
This project loads assets using relative paths. That means:

Your working directory must include the assets/ folder

If you see "model not found" or missing textures:

Copy assets/ into your build output folder (e.g. out/build/x64-Debug/assets)

OR configure the working directory in Visual Studio to be the root project folder

### 🎮 Controls
Key	Action
← / →	Decrease / Increase carousel rotation speed
C	Toggle camera mode (Free-Roam / Mounted Viewpoints)
WASD	Move camera (Free-Roam mode only)
Mouse	Look around (Free-Roam mode only)
Alt+f4 to close or simply Win key and then click on the X at the top-left corner

### 🧠 Notes

If motion appears too fast in windowed mode, toggle fullscreen manually using Alt + Enter or maximize the window

### 👤 Author
Created by Maximo Sánchez with downloaded assets from sketchfab.

