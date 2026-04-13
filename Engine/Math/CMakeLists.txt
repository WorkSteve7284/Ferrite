add_library(FerriteEngine-Math)

option(FERRITE_USE_DOUBLE "Use double precision for math" ON)

target_sources(FerriteEngine-Math

    PUBLIC
        FILE_SET CXX_MODULES
        FILES

        Math.cppm

        Vector/Vector.cppm
        Vector/Vector3.cppm
        Vector/Vector2.cppm
)

if (FERRITE_USE_DOUBLE)
    target_compile_definitions(FerriteEngine-Math PRIVATE "FERRITE_USE_DOUBLE")
endif()
