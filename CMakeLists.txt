cmake_minimum_required(VERSION 3.20)
project(explo)

set(CMAKE_CXX_STANDARD 20)

add_library(explo_lib
    src/input/EntityController.cpp
    src/input/EntityController.hpp

    src/video/DebugUi.cpp
    src/video/DebugUi.hpp
    src/video/Renderer.cpp
    src/video/Renderer.hpp

    src/util/camera.cpp
    src/util/camera.hpp
    src/util/CircularImage3d.hpp
    src/util/misc.cpp
    src/util/misc.hpp
    src/util/profile_stats.cpp
    src/util/profile_stats.hpp
    src/util/SyncJobExecutor.cpp
    src/util/SyncJobExecutor.hpp
    src/util/system.cpp
    src/util/system.hpp
    src/util/ThreadPool.cpp
    src/util/ThreadPool.hpp

    src/world/surface/BlockySurfaceGenerator.cpp
    src/world/surface/BlockySurfaceGenerator.hpp
    src/world/surface/SurfaceGenerator.hpp
    src/world/surface/Surface.hpp
    src/world/surface/SurfaceWriter.cpp
    src/world/surface/SurfaceWriter.hpp
    src/world/volume/storage/OctreeVolumeStorage.cpp
    src/world/volume/storage/OctreeVolumeStorage.hpp
    src/world/volume/storage/VolumeStorage.hpp
    src/world/volume/SinCosVolumeGenerator.cpp
    src/world/volume/SinCosVolumeGenerator.hpp
    src/world/BlockRegistry.cpp
    src/world/BlockRegistry.hpp
    src/world/Chunk.cpp
    src/world/Chunk.hpp
    src/world/DeltaChunkIterator.cpp
    src/world/DeltaChunkIterator.hpp
    src/world/Entity.cpp
    src/world/Entity.hpp
    src/world/World.cpp
    src/world/World.hpp

    src/Game.cpp
    src/Game.hpp
    src/log.hpp
    src/GlfwWindow.hpp
    src/GlfwWindow.cpp

    src/util/VirtualAllocator.cpp
    src/util/VirtualAllocator.hpp
    src/world/WorldView.cpp
    src/world/WorldView.hpp
    src/world/BlockData.hpp
    src/video/RenderApi.hpp
    src/video/RenderApi.cpp
    src/util/VirtualAllocator.cpp
    src/video/BakedWorldView.hpp
    src/video/BakedWorldView.cpp
    src/video/DeviceBuffer.hpp
    src/video/DeviceBuffer.cpp
    src/video/DeviceImage3d.hpp
    src/video/DeviceImage3d.cpp
    src/util/JobChain.cpp
    src/video/pipeline/CullWorldView.hpp
    src/video/pipeline/CullWorldView.cpp
    src/video/pipeline/DrawChunkList.hpp
    src/video/pipeline/DrawChunkList.cpp
    src/util/Aabb.cpp
    )

target_include_directories(explo_lib PUBLIC "./src/")

# ------------------------------------------------------------------------------------------------
# vren
# ------------------------------------------------------------------------------------------------

# vren
add_subdirectory(external/vren)

target_compile_definitions(vren PRIVATE VREN_LOG_LEVEL=VREN_LOG_LEVEL_WARN)

target_include_directories(explo_lib PUBLIC "external/vren/vren")
target_link_libraries(explo_lib PUBLIC vren)

include("external/vren/VRen.cmake")

setup_resources(explo_lib)

# TODO create a function for explo resources so that explo, explo_test can use same code
compile_shader(SHADERS "${CMAKE_CURRENT_LIST_DIR}/resources/shaders/cull_world_view.comp" "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/cull_world_view.comp.spv")

add_custom_target(explo_shaders DEPENDS ${SHADERS})
add_dependencies(explo_lib explo_shaders)

# ------------------------------------------------------------------------------------------------
# ImGui GLFW backend binary
# ------------------------------------------------------------------------------------------------

set(IMGUI_BACKENDS_BASE_URL https://raw.githubusercontent.com/ocornut/imgui/${imgui_TAG}/backends)

set(IMGUI_GLFW_BACKEND_DIR ${CMAKE_CURRENT_BINARY_DIR}/imgui_glfw_backend)

file(DOWNLOAD ${IMGUI_BACKENDS_BASE_URL}/imgui_impl_glfw.cpp ${IMGUI_GLFW_BACKEND_DIR}/imgui_impl_glfw.cpp)
file(DOWNLOAD ${IMGUI_BACKENDS_BASE_URL}/imgui_impl_glfw.h ${IMGUI_GLFW_BACKEND_DIR}/imgui_impl_glfw.h)

add_library(explo_imgui_glfw_backend
    ${IMGUI_GLFW_BACKEND_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_GLFW_BACKEND_DIR}/imgui_impl_glfw.h
    )

# ImGui
target_link_libraries(explo_imgui_glfw_backend PUBLIC imgui)

# glfw3
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(explo_imgui_glfw_backend PUBLIC glfw)

# ------------------------------------------------------------------------------------------------
# Third-party
# ------------------------------------------------------------------------------------------------

# glfw3
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(explo_lib PUBLIC glfw)

# glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(explo_lib PUBLIC glm::glm)

# entt
find_package(EnTT CONFIG REQUIRED)
target_link_libraries(explo_lib PUBLIC EnTT::EnTT)

# ImGui GLFW backend
target_include_directories(explo_lib PRIVATE ${IMGUI_GLFW_BACKEND_DIR})
target_link_libraries(explo_lib PUBLIC explo_imgui_glfw_backend)

# ------------------------------------------------------------------------------------------------
# explo
# ------------------------------------------------------------------------------------------------

add_executable(explo
    src/main.cpp
    )

target_link_libraries(explo PRIVATE explo_lib)

# ------------------------------------------------------------------------------------------------

add_subdirectory(test)
