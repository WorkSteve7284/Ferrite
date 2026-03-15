# Create target & project

project(FerriteEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CXX_MODULE_STD ON)

# Compile & link modules

# Core module
add_subdirectory(Core)

# Rendering
add_subdirectory(Rendering)
