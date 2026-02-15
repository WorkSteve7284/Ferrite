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
        Classes/Component.cppm
        Classes/Object.cppm

        # Exceptions
        Exceptions/Exceptions.cppm
        Exceptions/NoComponentFound.cppm
        Exceptions/NoObjectFound.cppm

        # Time
        Time/Time.cppm
        Time/TimeVariables.cppm
        Time/Timer.cppm

        # Debugging
        Debug/Debug.cppm
)

target_compile_definitions(FerriteEngine-Core PRIVATE FERRITE_DETERMINISTIC)
target_compile_definitions(FerriteEngine-Core PRIVATE DEBUG)
target_compile_definitions(FerriteEngine-Core PUBLIC FERRITE_EXCEPTIONS_ALLOWED)
