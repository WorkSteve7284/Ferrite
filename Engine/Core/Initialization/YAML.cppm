module;

#include <cstdint>
#include <type_traits>
#include <string_view>
#include <string>
#include <variant>
#include <vector>
#include <charconv>
#include <array>

export module Ferrite.Core.Initialization:YAML;

import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;

import :ConstexprTypes;

namespace Ferrite::Core::Initialization {

    export struct YAMLElement {

        using Value = std::variant<int, double, bool, constexpr_string<64>>;
        using AllTypes = std::variant<Value, constexpr_vector<std::size_t, 64>, constexpr_vector<Value, 64>>;

        constexpr YAMLElement() = default;
        constexpr YAMLElement(const YAMLElement&) = default;
        constexpr YAMLElement& operator=(const YAMLElement& other) = default;
        constexpr YAMLElement(constexpr_string<64> k) : key(k) {}
        constexpr YAMLElement(constexpr_string<64> k, AllTypes v) : key(k), value(v) {}

        constexpr_string<64> key;
        AllTypes value;
    };

    export template <std::size_t N> struct YAML {
        std::array<YAMLElement, N> arr;
    };

    enum class IndentLevel {
        UNKNOWN,
        SEQUENCE,
        VALUE_SEQUENCE,
        MAPPING
    };

    struct Frame {
        IndentLevel type;
        std::size_t indent;
    };

    /* Convert a YAML file represented as `std::array<char, N>` to YAML */
    export template <auto chars> consteval auto parse_yaml();
    /* Convert a YAML file represented as an array of string_views to YAML */
    export template <std::size_t N> constexpr auto parse_yaml(const std::array<constexpr_string<256>, N>);
    /* Parse the value of YAML to an actual value */
    constexpr YAMLElement::Value parse_string(const std::string_view);
    /* Parse an entire line */
    constexpr void parse_line(constexpr_string<256>&, constexpr_vector<YAMLElement, 256>&, constexpr_vector<std::uint8_t, 128>&);
    /* Convert an array of chars into an array of string_views. */
    template <std::size_t N, const std::array<char, N>& chars> consteval auto convert_to_strings();

    export template <auto chars> consteval auto parse_yaml() {

        static_assert(is_std_array<std::remove_cvref_t<decltype(chars)>>::value);

        constexpr auto sv_arr = convert_to_strings<chars.size(), chars>();

        constexpr auto p = parse_yaml(sv_arr);

        constexpr auto v = p.first;
        constexpr auto n = p.second;

        YAML<n> out;

        constexpr_copy(v, out.arr);

        return out;
    }

    export template <std::size_t N> constexpr auto parse_yaml(const std::array<constexpr_string<256>, N> strs) {

        std::vector<constexpr_string<256>> normalized(strs.begin(), strs.end());

        // Remove empty lines
        normalized.erase(std::remove_if(normalized.begin(), normalized.end(), [](std::string_view str) {
            return str.find_first_not_of(" \n\t") == str.npos;
        }), normalized.end());

        constexpr_vector<IndentLevel, 128> stack{}; // Essentially stores indentation level

        constexpr_vector<YAMLElement, 256> elements{};

        for (auto& str : normalized) {

            auto indent = str.view().find_first_not_of(' ');

            if (indent == std::string_view::npos)
                break;

            if (indent == indents.back()) {
                // Strip whitespace
                str.erase(0, indent);

                if (containers.empty()) {
                    // Parse new object

                    auto colon = str.view().find_first_of(':');

                    if (colon == std::string::npos) {
                        continue;
                    }

                    // Unknown value-sequence, sequence, or mapping
                    if (colon == str.view().find_last_not_of(' ')) {
                        containers.push_back(IndentLevel::UNKNOWN);
                        elements.emplace_back(make_string(str.substr(0, colon)));
                    }
                    else {
                        elements.emplace_back(make_string(str.substr(0, colon)), parse_string(str.substr(colon)));
                    }
                }
                else {
                    // If unknown, determine type
                    if (containers.back() == IndentLevel::UNKNOWN) {
                        if (str.view().starts_with("- ")) {
                            if (str.view().find_first_of(':') != std::string::npos) {
                                containers.back() = IndentLevel::SEQUENCE;
                                elements.back().value = YAMLElement::AllTypes{constexpr_vector<std::size_t, 64>{}};
                            }
                            else {
                                containers.back() = IndentLevel::VALUE_SEQUENCE;
                                elements.back().value = YAMLElement::AllTypes{constexpr_vector<YAMLElement::Value, 64>{}};
                            }
                        }
                        else {
                            containers.back() = IndentLevel::MAPPING;
                            elements.back().value = YAMLElement::AllTypes{constexpr_vector<std::size_t, 64>{}};
                        }
                    }

                    parse_line(str, elements, containers);
                }
            }
            if (indent > containers.size() * 2) {
                continue;
            }
            else if (indent != 0) {

                str.erase(0, indent);

                // Move back up the correct amount of indents
                containers.resize(indent/2);

                parse_line(str, elements, containers);
            }
            else {
                auto colon = str.view().find_first_of(':');

                if (colon == std::string::npos) {
                    continue;
                }

                // Unknown value-sequence, sequence, or mapping
                if (colon == str.view().find_last_not_of(' ')) {
                    containers.push_back(IndentLevel::UNKNOWN);
                    elements.emplace_back(make_string(str.substr(0, colon)));
                }
                else {
                    elements.emplace_back(make_string(str.substr(0, colon)), parse_string(str.substr(colon)));
                }
            }

        }

        return std::pair(elements, elements.size());

    }

    constexpr YAMLElement::Value parse_string(const std::string_view str) {

        // Trim whitespace
        std::string string{str.substr(str.find_first_not_of(' '), str.find_last_not_of(' ') - str.find_first_not_of(' '))};

        // Boolean
        if (string == "true" || string == "false") {
            return string == "true" ? true : false;
        }

        // Integer
        if (string.find_first_not_of("0123456789+-xABCDEFb") == std::string::npos) {
            int v;
            auto err = std::from_chars(string.data(), string.data() + string.size(), v);
            if (err.ec == std::errc{})
                return v;
        }

        if (string.find_first_not_of("0123456789+-.e") == std::string::npos) {
            double v;
            auto err = std::from_chars(string.data(), string.data() + string.size(), v);
            if (err.ec == std::errc{})
                return v;
        }

        // String
        // remove "..." and '...'

        // Remove "..."
        if (string.front() == '\"' && string.back() == '\"') {
            if (string[string.size() - 2] != '\\') {
                string = string.substr(1, string.size() - 2);

                for (int i = 0; i < string.size() - 1; i++) {
                    if (string[i] == '\\') {
                        switch (string[i+1]) {
                            case 'n':
                                string[i] = '\n';
                                string.erase(i+1, 1);
                                break;
                            case 't':
                                string[i] = '\t';
                                string.erase(i+1, 1);
                                break;
                            case '\\':
                                // remove last character
                                string.erase(i+1, 1);
                                break;
                            case '\"':
                                string.erase(i, 1);
                        }
                    }
                }
            }
        }

        // Remove '...'
        if (string.front() == '\'' && string.back() == '\'') {
            if (string[string.size() - 2] != '\'') {
                string = string.substr(1, string.size() - 2);

                for (std::size_t i = 0; i < string.size() - 1; i++) {
                    if (string[i] == '\'' && string[i+1] == '\'')
                        string.erase(i+1, 1);
                }
            }
        }

        return make_string(string);
    }

    constexpr void parse_line(constexpr_string<256>& line, constexpr_vector<YAMLElement, 256>& elements, constexpr_vector<IndentLevel, 128>& containers) {

        YAMLElement* whence = &elements.back();

        for (std::size_t i = 0; i < containers.size(); i++) {
            auto& vec = std::get<1>(whence->value);
            if (!vec.empty())
                whence = &elements[vec.back()];
            else {
                break;
            }
        }

        restart_switch:

        switch (containers.back()) {
            case IndentLevel::VALUE_SEQUENCE:

            line.erase(0, 2);

            if (whence->value.index() == 2) {
                std::get<2>(whence->value).emplace_back(parse_string(line));
            }
            break;

            case IndentLevel::SEQUENCE:
            {
                line.erase(0, 2);

                if (whence->value.index() != 1) break;

                auto colon = line.view().find_first_of(':');

                if (colon == std::string::npos) {
                    return;
                }

                // Unknown value-sequence, sequence, or mapping
                if (colon == line.view().find_last_not_of(' ')) {
                    containers.push_back(IndentLevel::UNKNOWN);
                    std::get<1>(whence->value).emplace_back(elements.size());
                    elements.emplace_back(make_string(line.substr(0, colon)));
                }
                else {
                    std::get<1>(whence->value).emplace_back(elements.size());
                    elements.emplace_back(make_string(line.substr(0, colon)), parse_string(line.substr(colon)));
                }
                break;
            }

            case IndentLevel::MAPPING:
            {
                line.erase(0, 2);

                if (whence->value.index() != 1) break;

                auto colon = line.view().find_first_of(':');

                if (colon == std::string::npos) {
                    return;
                }

                // Unknown value-sequence, sequence, or mapping
                if (colon == line.view().find_last_not_of(' ')) {
                    containers.push_back(IndentLevel::UNKNOWN);
                    std::get<1>(whence->value).emplace_back(elements.size());
                    elements.emplace_back(make_string(line.substr(0, colon)));
                }
                else {
                    std::get<1>(whence->value).emplace_back(elements.size());
                    elements.emplace_back(make_string(line.substr(0, colon)), parse_string(line.substr(colon)));
                }
                break;
            }

            case IndentLevel::UNKNOWN:
                // Determine type
                if (line.view().starts_with("- ")) {
                    if (line.view().find_first_of(':') != std::string::npos) {
                        containers.back() = IndentLevel::SEQUENCE;
                        whence->value = YAMLElement::AllTypes{constexpr_vector<std::size_t, 64>{}};
                    }
                    else {
                        containers.back() = IndentLevel::VALUE_SEQUENCE;
                        whence->value = YAMLElement::AllTypes{constexpr_vector<YAMLElement::Value, 64>{}};
                    }
                }
                else {
                    containers.back() = IndentLevel::MAPPING;
                    whence->value = YAMLElement::AllTypes{constexpr_vector<std::size_t, 64>{}};
                }

                goto restart_switch;
        }
    }

    template <std::size_t N, const std::array<char, N>& chars> consteval auto convert_to_strings() {
        std::array<constexpr_string<256>, std::count(chars.begin(), chars.end(), '\n')> arr;

        // Copy into array
        auto last_newline = chars.begin();

        for (std::size_t i = 0; i < arr.size(); i++) {
            auto next_newline = std::find(last_newline, chars.end(), '\n');

            // Copy characters (ignore \r and errant \n)
            for (auto it = last_newline; it != next_newline; it++) {
                if (*it != '\r' && *it != '\n')
                    arr[i].chars[arr[i].size++] = *it;
            }

            if (next_newline == chars.end()) {
                break;
            }
            else {
                last_newline = next_newline;
            }
        }

        // Trim comments & simplify escape stuff
        for (std::size_t i = 0; i < arr.size(); i++) {
            std::uint8_t escaped = 0; // 0 = none, 1 = single, 2 = double
            for (std::size_t j = 0; j < arr[i].size; j++) {
                switch (arr[i][j]) {
                    case '#':
                        if (escaped == 0)
                            arr[i].erase(j);
                        break;
                    case '\'':
                        if (escaped == 0) {
                            if (j+1 < arr[i].size && arr[i][j+1] == '\'') arr[i].erase(j+1, 1);
                            else escaped = 1;
                        }
                        else if (escaped == 1) {
                            if (j+1 < arr[i].size && arr[i][j+1] == '\'') arr[i].erase(j+1, 1);
                            else escaped = 0;
                        }
                        break;

                    case '"':
                        if (escaped == 0)
                            escaped = 2;
                        else if (escaped == 2) {
                            if (j > 0 && arr[i][j-1] == '\\') arr[i].erase(j-1, 1);
                            else escaped = 0;
                        }
                        break;

                    case '\\':
                        if (escaped == 2) {
                            if (j+1 < arr[i].size) {
                                switch (arr[i][j+1]) {
                                    case 'n':
                                        arr[i][j] = '\n';
                                        arr[i].erase(j+1, 1);
                                    case 't':
                                        arr[i][j] = '\t';
                                        arr[i].erase(j+1, 1);
                                    case '\\':
                                        arr[i][j] = '\\';
                                        arr[i].erase(j+1, 1);
                                }
                            }
                        }
                        break;

                }
            }
        }

        return arr;
    }

}
