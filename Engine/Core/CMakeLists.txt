# Core module for Ferrite

add_library(FerriteEngine-Core)

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

target_compile_definitions(FerriteEngine-Core PRIVATE FERRITE_DETERMINISTIC)
target_compile_definitions(FerriteEngine-Core PRIVATE FERRITE_EXCEPTIONS_ALLOWED)
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_FIXED_FRAMERATE=60")
target_compile_definitions(FerriteEngine-Core PRIVATE "FERRITE_DEBUG_LOG_FILE=\"\"")
