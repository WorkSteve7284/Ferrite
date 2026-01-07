#include <string>
#include <variant>
#include <vector>
export module Ferrite.Core.Initialization:YAML;

import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;

namespace Ferrite::Core::Initialization {

    struct YAMLElement {

        enum class Type {
            MAPPING,
            SEQUENCE,
            PAIR
        } type;

        using Value = std::variant<int, double, bool, std::string>;

        std::string key;
        std::variant<Value, std::variant<std::vector<YAMLElement>, std::vector<Value>>> value;
    };

    /*

    Read YAML pipeline:

    Get file (somehow) as vector<char>

    normalize newlines (remove \r)

    Parse
        Go through each line
        Find first line which doesn't start with whitespace
        key = before colon
        if nothing after colon
            look at next line
            if next line starts with two spaces
                add indent to counter
                check if line starts with ' -'
                    if sequence is to a value
                        add value
                        look at next line
                parse this line like the rest

    */

    consteval std::vector<YAMLElement> parse_yaml(std::vector<std::string>&& vec) {

        auto normalized = vec;

        // Remove empty lines & comments

        enum class Escaped {
            NO,
            SINGLE,
            DOUBLE
        };

        for (std::size_t i = 0; i < normalized.size(); i++) {
            if (normalized[i].empty()) {
                normalized.erase(normalized.begin() + i);
                i--;
            }
            else {

                auto first = normalized[i].find_first_of("\'\"#");

                if (first == std::string::npos) [[likely]] continue;

                Escaped escape = Escaped::NO;
                for (std::size_t j = first; j < normalized[i].size(); j++) {
                    switch (vec[i][j]) {
                        case '\"':
                            // Ensure not escaped
                            if (j == 0 || !(j != 0 && normalized[i][j-1] == '\\')) [[unlikely]]
                                break;
                            else
                            {
                                if (escape == Escaped::NO) escape = Escaped::DOUBLE;
                                else if (escape == Escaped::DOUBLE) escape = Escaped::NO;
                            }

                            break;
                        case '\'':
                            // Ensure not escaped
                            if (j == 0 || !(j != 0 && normalized[i][j-1] == '\'')) [[unlikely]]
                                break;
                            else
                            {
                                if (escape == Escaped::NO) escape = Escaped::SINGLE;
                                else if (escape == Escaped::DOUBLE) escape = Escaped::NO;
                            }

                            break;
                        case '#':
                            if (escape == Escaped::NO)
                                normalized[i].erase(j);
                            break;
                    }
                }
            }
        }

        // Parse YAML

        std::size_t current_indent = 0;

        for (auto& str : normalized) {
            std::string prefix(current_indent, ' ');
            if (str.starts_with(prefix)) {
                // Parse

                // strip whitespace

            } else {
                // Move up (look at elements of map?)
            }

        }
    }

}
