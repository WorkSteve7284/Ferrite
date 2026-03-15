# Core module for Ferrite

add_library(FerriteEngine-Core)

option(FERRITE_EXCEPTIONS_ALLOWED "Allow exceptions to be thrown" OFF)
option(FERRITE_PREVENT_LEAK "Prevent memory leaks whem moving objects" ON)
option(FERRITE_DETERMINISTIC "Only allow one thread to work at a time (emulate single-threaded behavior, mostly)" OFF)
set(FERRITE_FIXED_FRAMERATE "60" CACHE STRING "How many frames per second fixed_updates run")
set(FERRITE_LOG_FILE "" CACHE STRING "File to write debug messages to")

target_sources(FerriteEngine-Core

    PUBLIC
        FILE_SET CXX_MODULES
        FILES

        Core.cppm

        Config.cppm

        Manager.cppm

        # Job system
        Jobs/Jobs.cppm
        Jobs/Worker.cppm
        Jobs/Queue.cppm

        # Classes
        Classes/Classes.cppm
        Classes/Prefabs.cppm
        Classes/PrefabManager.cppm
        Classes/Component.cppm
        Classes/Object.cppm

        # Exceptions
        Exceptions/Exceptions.cppm
        Exceptions/NoComponentFound.cppm
        Exceptions/NoObjectFound.cppm
        Exceptions/CircularStructure.cppm

        # Time
        Time/Time.cppm
        Time/TimeVariables.cppm
        Time/Timer.cppm

        # Debugging
        Debug/Debug.cppm

        # Threading
        Threads/Threads.cppm

        # Event System
        Events/Events.cppm
    PRIVATE
        Classes/Object.cpp

)

if(FERRITE_EXCEPTIONS_ALLOWED)
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_EXCEPTIONS_ALLOWED")

endif()

if(FERRITE_PREVENT_LEAK)
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_PREVENT_LEAK")
endif()

if(FERRITE_DETERMINISTIC)
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_DETERMINISTIC")
endif()

target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_DEBUG_LOG_FILE=\"${FERRITE_LOG_FILE}\"")
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_FIXED_FRAMERATE=${FERRITE_FIXED_FRAMERATE}")
