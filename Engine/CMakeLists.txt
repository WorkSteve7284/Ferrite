# Create target & project

project(Ferrite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CXX_MODULE_STD ON)

add_library(Ferrite INTERFACE)


# Compile & link modules

# Core module
add_subdirectory(Core)
target_link_libraries(Ferrite INTERFACE FerriteEngine-Core)

# Rendering
#add_subdirectory(Rendering)
#target_link_libraries(Ferrite INTERFACE FerriteEngine-Rendering)
