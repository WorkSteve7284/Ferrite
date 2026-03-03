module;

#include <cstddef>

export module Ferrite.Core.Config;

namespace Ferrite::Core::Config {

    export constexpr std::size_t MAX_THREADS = 32;

    export constexpr double FIXED_DELTA_TIME = (1.0/60.0);

    export constexpr unsigned char UPDATE_PRIORITY = 0;
    export constexpr unsigned char FIXED_UPDATE_PRIORITY = 3;

    #ifdef FERRITE_EXCEPTIONS_ALLOWED
        export constexpr bool EXCEPTIONS_ALLOWED = true;
    #else
        export constexpr bool EXCEPTIONS_ALLOWED = false;
    #endif

    #ifdef FERRITE_DETERMINISTIC
        export constexpr bool DETERMINISTIC = true;
    #else
        export constexpr bool DETERMINISTIC = false;
    #endif

    #ifdef NDEBUG
        export constexpr bool FERRITE_DEBUG = false;
    #else
        export constexpr bool FERRITE_DEBUG = true;
    #endif

    export constexpr char FERRITE_DEBUG_LOG[] = "";
}  // namespace Ferrite::Core::Config
