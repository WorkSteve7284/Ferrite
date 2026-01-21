module;

#include <string_view>
#include <format>
#include <type_traits>
#include <optional>
#include <string>
#include <stdexcept>
#include <array>
#include <vector>

export module Ferrite.Core.Initialization:ConstexprTypes;

namespace Ferrite::Core::Initialization {

    /* Constexpr-capable vector type, with fixed capacity. */
    export template <typename T, std::size_t N>
    struct constexpr_vector {
    private:
        // Properly aligned raw storage
        std::array<std::optional<T>, N> _data;
        std::size_t _size = 0;

    public:
        /* ===================== Access ===================== */

        constexpr T& at(std::size_t i) {
            if (i >= _size) throw std::out_of_range("constexpr_vector::at");
            return *_data[i];
        }

        constexpr const T& at(std::size_t i) const {
            if (i >= _size) throw std::out_of_range("constexpr_vector::at");
            return *_data[i];
        }

        constexpr T& operator[](std::size_t i) noexcept {
            return *_data[i];
        }

        constexpr const T& operator[](std::size_t i) const noexcept {
            return *_data[i];
        }

        constexpr T* data() noexcept {
            return &_data[0];
        }

        constexpr const T* data() const noexcept {
            return &_data[0];
        }

        constexpr T& front() noexcept {
            return *_data[0];
        }

        constexpr const T& front() const noexcept {
            return *_data[0];
        }

        constexpr T& back() noexcept {
            return *_data[_size - 1];
        }

        constexpr const T& back() const noexcept {
            return *_data[_size - 1];
        }

        /* ===================== Iterators ===================== */

        constexpr T* begin() noexcept { return _data.begin(); }
        constexpr const T* cbegin() const noexcept { return _data.cbegin(); }
        constexpr T* rbegin() noexcept { return _data.rbegin(); }
        constexpr const T* crbegin() const noexcept { return _data.crbegin(); }

        constexpr T* end() noexcept { return _data.end(); }
        constexpr const T* cend() const noexcept { _data.cend(); }
        constexpr T* rend() noexcept { return _data.rend(); }
        constexpr const T* crend() const noexcept { return _data.crend(); }

        /* ===================== Capacity ===================== */

        constexpr bool empty() const noexcept { return _size == 0; }
        constexpr std::size_t size() const noexcept { return _size; }
        constexpr std::size_t capacity() const noexcept { return N; }
        constexpr std::size_t max_size() const noexcept { return N; }

        /* ===================== Modifiers ===================== */

        constexpr void clear() noexcept {
            for (std::size_t i = 0; i < _size; ++i) {
                _data[i] = std::nullopt;
            }
            _size = 0;
        }

        constexpr void push_back(const T& value) {
            if (_size >= N) throw std::length_error("constexpr_vector full");
            _data[_size++] = value;
        }

        constexpr void push_back(T&& value) {
            if (_size >= N) throw std::length_error("constexpr_vector full");
            _data[_size++] = value;
        }

        template <typename... Args>
        constexpr T& emplace_back(Args&&... args) {
            if (_size >= N) throw std::length_error("constexpr_vector full");
            _data[_size++] = T{std::forward<Args&&>(args)...};

            return back();
        }

        constexpr void pop_back() {
            if (_size == 0) return;
            --_size;
            _data[_size] = std::nullopt;
        }

        constexpr void resize(std::size_t count) {
            if (count > N) throw std::length_error("constexpr_vector::resize");

            if (count < _size) {
                for (std::size_t i = count; i < _size; ++i) {
                    _data[i] = std::nullopt;
                }
            } else {
                for (std::size_t i = _size; i < count; ++i) {
                    _data[i] = T{};
                }
            }

            _size = count;
        }

        constexpr void resize(std::size_t count, const T& value) {
            if (count > N) throw std::length_error("constexpr_vector::resize");

            if (count < _size) {
                for (std::size_t i = count; i < _size; ++i) {
                    _data[i] = std::nullopt;
                }
            } else {
                for (std::size_t i = _size; i < count; ++i) {
                    _data[i] = value;
                }
            }

            _size = count;
        }

        constexpr operator std::vector<T>() {
            return std::vector<T>(_data.begin(), _data.end());
        }
    };

    export template <std::size_t N> struct constexpr_string {
        std::array<char, N> chars{};
        std::size_t size = 0;

        constexpr std::string_view view() const {
            return std::string_view(chars.begin(), chars.begin() + size);
        }

        constexpr std::string_view substr(std::size_t start, std::size_t length) {
            return std::string_view(chars.begin() + start, chars.begin() + start + length);
        }
        constexpr std::string_view substr(std::size_t start) {
            return std::string_view(chars.begin() + start, chars.begin() + size);
        }

        constexpr char& operator[](std::size_t i) {
            if (i < size)
                return chars[i];
            else throw std::out_of_range(std::format("constexpr_string: Index {} out of bounds!", i));
        }

        constexpr void erase(std::size_t pos) {
            size = pos;
        }

        constexpr void erase(std::size_t pos, std::size_t length) {
            for (int i = pos; i < size && i+length < N; i++) {
                chars[i] = chars[i + length];
            }
            size -= length;
        }

        constexpr operator std::string() const {
            return std::string(chars.begin(), chars.end());
        }
        constexpr operator std::string_view() const {
            return std::string_view(chars.begin(), chars.end());
        }
    };

    constexpr constexpr_string<64> make_string(const std::string& str) {
        constexpr_string<64> out;
        if (str.size() < 64) {
            std::copy(str.begin(), str.end(), out.chars.begin());
            out.size = str.size();
        }
        else {
            std::copy(str.begin(), str.begin() + 64, out.chars.begin());
            out.size = 64;
        }
        return out;
    }

    constexpr constexpr_string<64> make_string(std::string_view str) {
        constexpr_string<64> out;
        if (str.size() < 64) {
            std::copy(str.begin(), str.end(), out.chars.begin());
            out.size = str.size();
        }
        else {
            std::copy(str.begin(), str.begin() + 64, out.chars.begin());
            out.size = 64;
        }
        return out;
    }

    template <typename A, typename B> constexpr void constexpr_copy(const A& from, B& to) {
        for (std::size_t i = 0; i < from.size() && i < to.size(); i++) {
            to[i] = from[i];
        }
    }

    template <typename> struct is_std_array : std::false_type {};
    template <typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
}

template <std::size_t N> struct std::formatter<Ferrite::Core::Initialization::constexpr_string<N>, char> : std::formatter<std::string_view, char> {
    template <typename Context> auto format(const Ferrite::Core::Initialization::constexpr_string<N>& s, Context& ctx) const {
        return std::formatter<std::string_view, char>::format(s.view(), ctx);
    }
};
