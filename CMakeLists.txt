cmake_minimum_required(VERSION 3.6)
project(nle)

#project variables
set(NLE_EDITOR_EXECUTABLE_NAME nle-editor)
set(NLE_SHARED_LIBRARY_NAME nle)
#project variables end

include_directories(src vendor)

set(
    NLE_SOURCE_INCLUDE_DIRS
    src/nle
)

set(
    NLE_VENDOR_INCLUDE_DIRS
    vendor
    vendor/imgui
    vendor/imgui/backends
    vendor/nlohmann
)

set(
    NLE_SOURCES
    src/nle/AABB.cpp
    src/nle/Camera.cpp
    src/nle/Globals.cpp
    src/nle/ImGUI_GLFW.cpp
    src/nle/InputHandler.cpp
    src/nle/Light.cpp
    src/nle/Material.cpp
    src/nle/Mesh.cpp
    src/nle/Model.cpp
    src/nle/MultiMesh.cpp
    src/nle/Nle.cpp
    src/nle/Object3D.cpp
    src/nle/PhysicsEngine.cpp
    src/nle/PhysicsObject3D.cpp
    src/nle/Renderer3D.cpp
    src/nle/RenderObject3D.cpp
    src/nle/Scene.cpp
    src/nle/Shader.cpp
    src/nle/Sky.cpp
    src/nle/Terrain.cpp
    src/nle/Texture.cpp
    src/nle/TextureFactory.cpp
    src/nle/Window.cpp
)
set(NLE_DEPENDENCIES -lglfw -lGL -lGLEW -lGLU -limgui -lstb)

# libnle
add_library("${NLE_SHARED_LIBRARY_NAME}" SHARED "${NLE_SOURCES}")
target_link_libraries("${NLE_SHARED_LIBRARY_NAME}" "${NLE_DEPENDENCIES}")
# target_include_directories("${NLE_SHARED_LIBRARY_NAME}" PRIVATE "${NLE_SOURCE_INCLUDE_DIRS}" "${NLE_VENDOR_INCLUDE_DIRS}")
target_include_directories("${NLE_SHARED_LIBRARY_NAME}" PRIVATE "${NLE_SOURCE_INCLUDE_DIRS}" src vendor vendor/imgui vendor/imgui/backends vendor/nlohmann)

# nle editor
set(NLE_EDITOR_PATH src/nle-editor.cpp)
set(NLE_EDITOR_DEPENDENCIES "${NLE_DEPENDENCIES}" "${NLE_SHARED_LIBRARY_NAME}")
add_executable("${NLE_EDITOR_EXECUTABLE_NAME}" "${NLE_EDITOR_PATH}")
target_link_libraries("${NLE_EDITOR_EXECUTABLE_NAME}" "${NLE_EDITOR_DEPENDENCIES}")
# target_include_directories("${NLE_EDITOR_EXECUTABLE_NAME}" PRIVATE "${NLE_SOURCE_INCLUDE_DIRS}" "${NLE_VENDOR_INCLUDE_DIRS}")
target_include_directories("${NLE_EDITOR_EXECUTABLE_NAME}" PRIVATE "${NLE_SOURCE_INCLUDE_DIRS}" src vendor vendor/imgui vendor/imgui/backends vendor/nlohmann)