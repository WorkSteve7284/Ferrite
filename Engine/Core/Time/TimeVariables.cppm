module;

#include <atomic>

export module Ferrite.Core.Time:TimeVariables;

namespace Ferrite::Core::Time {
    export std::atomic<double> runtime;
} // Ferrite::Core::Time
