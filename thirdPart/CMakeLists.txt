
FILE (GLOB ALL_SOURCES *.cpp *.c *.cc *.h *.hpp *.inl *.inc)
FILE (GLOB ALL_INCLUDES *.h *.hpp *.inl *.inc)

source_group("Header Files" FILES ${ALL_INCLUDES})

aux_source_directory(./glm  DIR_LIB_SRCS)

add_library(glm ${DIR_LIB_SRCS})
#add_library(Glfw ${DIR_LIB_SRCS})

#target_link_libraries(demo )
