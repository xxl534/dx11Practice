cmake_minimum_required (VERSION 2.6)
project(DX11Practice) 
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET(COMMON_INCLUDE_DIR ./;)

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

else( CMAKE_SIZEOF_VOID_P EQUAL 8 )

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
SET(LINK_LIB_DEBUG "d3d11.lib" "D3DCompiler.lib" "Effects11d.lib" "dxgi.lib" "dxguid.lib" "DirectXTK.lib")
SET(LINK_LIB_RELEASE "d3d11.lib" "D3DCompiler.lib" "Effects11.lib" "dxgi.lib" "dxguid.lib" "DirectXTK.lib")
INCLUDE(CMakeMacro.txt)
INCLUDE(CMake_PreCompiled.txt)

ADD_SUBDIRECTORY(Common)

ADD_SUBDIRECTORY(Chapter/6/Box)
ADD_SUBDIRECTORY(Chapter/lighting_7/lighting)
ADD_SUBDIRECTORY(Chapter/texture_8/crate)
ADD_SUBDIRECTORY(Chapter/texture_8/RotateTextureCrate)
ADD_SUBDIRECTORY(Chapter/texture_8/FrameAnimCrate)

ADD_SUBDIRECTORY(Chapter/blending_9/blending)

ADD_SUBDIRECTORY(Chapter/stencil_10/mirror)
ADD_SUBDIRECTORY(Chapter/stencil_10/cylinder)

ADD_SUBDIRECTORY(Chapter/geometry_11/treeBillboard)
ADD_SUBDIRECTORY(Chapter/geometry_11/Explosion)

ADD_SUBDIRECTORY(Chapter/compute_12/blur)
ADD_SUBDIRECTORY(Chapter/compute_12/compute)
ADD_SUBDIRECTORY(Chapter/compute_12/wave)

ADD_SUBDIRECTORY(Chapter/tessellation_13/basicTessellation)
ADD_SUBDIRECTORY(Chapter/tessellation_13/bezierPatch)
ADD_SUBDIRECTORY(Chapter/tessellation_13/icosahedron)

ADD_SUBDIRECTORY(Chapter/camera_14/camera)
