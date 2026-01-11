module;

#include <string_view>
#include <string>
#include <variant>
#include <vector>
#include <charconv>
#include <array>

export module Ferrite.Core.Initialization:YAML;

import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;

namespace Ferrite::Core::Initialization {

    export template <typename T, std::size_t N> struct constexpr_vector {
    private:
        std::array<T, N> data;
        std::size_t _size = 0;
    public:

        constexpr std::size_t size() const noexcept { return _size; }

        constexpr T& back() noexcept { return data[_size - 1]; }

        constexpr T& operator[](std::size_t i) {
            return data[i];
        }

        constexpr T& push_back(const T& elem) {
            data[_size++] = elem;
            return back();
        }

        template <typename... Args> constexpr T& emplace_back(Args&&... args) {
            data[_size++] = T{args...};
            return back();
        }

        constexpr auto begin() noexcept {
            return data.begin();
        }
        constexpr auto end() noexcept {
            return data.end();
        }
    };

    export template <std::size_t N> struct constexpr_string {
    private:
        std::string_view view;
        std::array<char, N> chars;
    public:
        constexpr constexpr_string(const std::string& str) {
            if (str.size() < N) {
                std::copy(str.begin(), str.end(), chars.begin());
                view = std::string_view(chars.begin(), chars.begin() + str.size());
            }
        }
        constexpr constexpr_string() {
            view = std::string_view(chars.begin(), chars.begin());
        }
    };

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

        return constexpr_string<64>(string);
    }

    export template <std::size_t N> constexpr auto parse_yaml(const std::array<std::string_view, N> strs) {

        std::vector<std::string> normalized(strs.begin(), strs.end());

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
                    switch (normalized[i][j]) {
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

        /*

        map = 1
        sequence (yaml element) = 2
        sequence (value) = 4

        sequence = 6

        */

        std::vector<unsigned char> containers;

        constexpr_vector<YAMLElement, 256> elements;

        for (auto& str : normalized) {

            std::string prefix(containers.size() * 2, ' ');

            if (str.starts_with(prefix)) {
                // Strip whitespace
                str.erase(0, containers.size() * 2);


                if (containers.empty()) {
                    // Parse new object

                    auto colon = str.find_first_of(':');

                    if (colon == std::string::npos) {
                        continue;
                    }

                    // Unknown value-sequence, sequence, or mapping
                    if (colon == str.find_last_not_of(' ')) {
                        containers.push_back(0b1000);
                        elements.emplace_back(str.substr(0, colon));
                    }
                    else {
                        elements.emplace_back(str.substr(0, colon), parse_string(str.substr(colon)));
                    }
                }
                else {
                    // If unknown, determine type
                    if (containers.back() == 8) {
                        if (str.starts_with("- ")) {

                            if (str.find_first_of(':') != std::string::npos)
                                containers.back() = 2;
                            else
                                containers.back() = 4;

                        }
                        else
                            containers.back() = 1;
                    }

                    // add to where?
                    YAMLElement* whence = &elements.back();

                    for (std::size_t i = 0; i < containers.size(); i++) {
                        whence = &elements[std::get<1>(whence->value).back()];
                    }

                    switch (containers.back()) {
                        case 4: // Value-Sequence

                        str.erase(0, 2);

                        if (whence->value.index() == 2) {
                            std::get<2>(whence->value).emplace_back(parse_string(str));
                        }
                        break;

                        case 2: // Sequence
                        {
                            str.erase(0, 2);

                            if (whence->value.index() != 1) break;

                            auto colon = str.find_first_of(':');

                            if (colon == std::string::npos) {
                                continue;
                            }

                            // Unknown value-sequence, sequence, or mapping
                            if (colon == str.find_last_not_of(' ')) {
                                containers.push_back(0b1000);
                                std::get<1>(whence->value).push_back(elements.size());
                                elements.emplace_back(str.substr(0, colon));
                            }
                            else {
                                std::get<1>(whence->value).push_back(elements.size());
                                elements.emplace_back(str.substr(0, colon), parse_string(str.substr(colon)));
                            }
                            break;
                        }

                        case 1: // Mapping
                        {
                            str.erase(0, 2);

                            if (whence->value.index() != 1) break;

                            auto colon = str.find_first_of(':');

                            if (colon == std::string::npos) {
                                continue;
                            }

                            // Unknown value-sequence, sequence, or mapping
                            if (colon == str.find_last_not_of(' ')) {
                                containers.push_back(0b1000);
                                std::get<1>(whence->value).emplace_back(elements.size());
                                elements.emplace_back(str.substr(0, colon));
                            }
                            else {
                                std::get<1>(whence->value).emplace_back(elements.size());
                                elements.emplace_back(str.substr(0, colon), parse_string(str.substr(colon)));
                            }
                            break;
                        }
                    }
                }
            }
            else {
                containers.pop_back();

                YAMLElement* whence = &elements.back();

                for (std::size_t i = 0; i < containers.size(); i++) {
                    whence = &elements[std::get<1>(whence->value).back()];
                }

                switch (containers.back()) {
                    case 4: // Value-Sequence

                    str.erase(0, 2);

                    if (whence->value.index() == 2) {
                        std::get<2>(whence->value).emplace_back(parse_string(str));
                    }
                    break;

                    case 2: // Sequence
                    {
                        str.erase(0, 2);

                        if (whence->value.index() != 1) break;

                        auto colon = str.find_first_of(':');

                        if (colon == std::string::npos) {
                            continue;
                        }

                        // Unknown value-sequence, sequence, or mapping
                        if (colon == str.find_last_not_of(' ')) {
                            containers.push_back(0b1000);
                            std::get<1>(whence->value).emplace_back(elements.size());
                            elements.emplace_back(str.substr(0, colon));
                        }
                        else {
                            std::get<1>(whence->value).emplace_back(elements.size());
                            elements.emplace_back(str.substr(0, colon), parse_string(str.substr(colon)));
                        }
                        break;
                    }

                    case 1: // Mapping
                    {
                        str.erase(0, 2);

                        if (whence->value.index() != 1) break;

                        auto colon = str.find_first_of(':');

                        if (colon == std::string::npos) {
                            continue;
                        }

                        // Unknown value-sequence, sequence, or mapping
                        if (colon == str.find_last_not_of(' ')) {
                            containers.push_back(0b1000);
                            std::get<1>(whence->value).emplace_back(elements.size());
                            elements.emplace_back(str.substr(0, colon));
                        }
                        else {
                            std::get<1>(whence->value).emplace_back(elements.size());
                            elements.emplace_back(str.substr(0, colon), parse_string(str.substr(colon)));
                        }
                        break;
                    }
                }
            }

        }

        return std::pair(elements, elements.size());
    }

    template <std::size_t N, const std::array<char, N>* chars> consteval auto convert_to_strings() {
        std::array<std::string_view, std::count(chars->begin(), chars->end(), '\n')> arr;

        // Create string views for characters between newlines, excluding '\r's

        auto last_newline = chars->begin();

        for (std::size_t i = 0; i < arr.size(); i++) {
            auto next_newline = std::find(last_newline, chars->end(), '\n');

            arr[i] = std::string_view(last_newline, next_newline);

            if (next_newline == chars->end()) {
                break;
            }
            else {
                last_newline = next_newline;
            }

        }

        return arr;
    }

    export template <std::size_t N, const std::array<char, N> chars> constexpr auto parse_yaml() {

        constexpr auto sv_arr = convert_to_strings<N, &chars>();

        constexpr auto p = parse_yaml(sv_arr);

        constexpr auto v = p.first;
        constexpr auto n = p.second;

        YAML<n> out;

        std::copy(v.begin(), v.end(), out.arr.begin());

        return out;
    }

}
