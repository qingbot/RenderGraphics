
cmake_minimum_required (VERSION 2.8)
FILE (GLOB_RECURSE ALL_SOURCES *.cpp *.c *.cc *.h *.hpp *.inl *.inc)
FILE (GLOB_RECURSE ALL_INCLUDES *.h )


# source_group("Header Files" FILES ${ALL_INCLUDES})
aux_source_directory(. DIR_SRCS)