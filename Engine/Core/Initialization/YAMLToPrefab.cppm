module;

#include <array>

export module Ferrite.Core.Initialization:YAMLToPrefab;

import :YAML;

namespace Ferrite::Core::Initialization {
    template <std::size_t N, const std::array<char, N>& chars> constexpr void generate_prefabs() {
        // Parse YAML
        constexpr auto yaml = parse_yaml<N, chars>();

        // Convert to prefabs


    }
}
