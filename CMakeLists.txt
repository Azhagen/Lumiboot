cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0083 NEW)
cmake_policy(SET CMP0079 NEW)

project("Lumiboot")
enable_language(C ASM)

set(CMAKE_C_STANDARD                    17)
set(CMAKE_C_EXTENSIONS                  ON)
set(CMAKE_C_EXTENSIONS_REQUIRED         TRUE)
set(CMAKE_C_VISIBILITY_PRESET           hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN     YES)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION  TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(bios)