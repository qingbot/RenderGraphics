
cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)
FILE (GLOB_RECURSE ALL_SOURCES Source/*.cpp Source/*.c Source/*.cc  Source/*.hpp Source/*.inl Source/*.inc)
FILE (GLOB_RECURSE ALL_INCLUDES Source/*.h )
#FILE(GLOB ALL_VULKAN_LIB thirdPart/VulkanSDK/1.3.275.0/Lib/*.lib)

#define VK_USE_PLATFORM_WIN32_KHR

source_group("Header Files" FILES ${ALL_INCLUDES})
source_group("Source" FILES ${ALL_SOURCES})
source_group("Vulkan" FILES ${ALL_VULKAN})

project(LearnVulkan)

add_subdirectory(Source)
add_subdirectory(thirdPart/glm)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source)
aux_source_directory(. DIR_SRCS)
aux_source_directory(./Source DIR_SRCS)


add_executable(${PROJECT_NAME} ${DIR_SRCS} ${ALL_INCLUDES} ${ALL_VULKAN} ${ALL_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WINDOWS)

target_include_directories(${PROJECT_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdPart/glfw/include)
target_include_directories(${PROJECT_NAME}  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdPart/VulkanSDK/1.3.275.0/Include)


target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdPart/VulkanSDK/1.3.275.0/Lib/vulkan-1.lib)
target_link_libraries(${PROJECT_NAME}  glm_static)
target_link_libraries(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/thirdPart/glfw/lib/glfw3.lib)
#target_link_libraries(${PROJECT_NAME} ${ALL_VULKAN_LIB})
