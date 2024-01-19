
cmake_minimum_required (VERSION 2.8)

project(LearnDX11)

add_subdirectory(Source)

aux_source_directory(. DIR_SRCS)

add_executable(demo ${DIR_SRCS})

target_link_libraries(demo DX11Content d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib winmm.lib d2d1.lib dwrite.lib)
