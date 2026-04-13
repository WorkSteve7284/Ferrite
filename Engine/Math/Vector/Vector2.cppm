module;

#include <stdexcept>
#include <type_traits>
#include <cmath>

export module Ferrite.Math.Vector.Vector2;

namespace Ferrite::Math::Vector {

    export template <typename T> struct Vector2 {
        T x, y;

        Vector2();
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2(U x_, U y_);
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2(const Vector2<U>&);
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2(Vector2<U>&&);

        T magnitude() const noexcept;
        T sqr_magnitude() const noexcept;
        Vector2 normalized() const;
        Vector2& normalize();

        static T dot(const Vector2& a, const Vector2& b);
        static Vector2 cross(const Vector2& a, const Vector2& b);

        static Vector2 zero() noexcept;
        static Vector2 one() noexcept;
        static Vector2 up() noexcept;
        static Vector2 down() noexcept;
        static Vector2 left() noexcept;
        static Vector2 right() noexcept;
        static Vector2 forward() noexcept;
        static Vector2 backward() noexcept;

        Vector2 operator+(const Vector2& other) const noexcept;
        Vector2 operator-(const Vector2& other) const noexcept;
        Vector2 operator-() const noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2 operator*(U scalar) const noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2 operator/(U scalar) const noexcept;

        Vector2& operator+=(const Vector2& other) noexcept;
        Vector2& operator-=(const Vector2& other) noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2& operator*=(U scalar) noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector2& operator/=(U scalar) noexcept;

        bool operator==(const Vector2& other) const noexcept;

        T& operator[](std::size_t i);
        T operator[](std::size_t i) const;
    };

    template <typename T, typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T> operator*(U scalar, const Vector2<T>& vec) noexcept {
        return vec * scalar;
    }

    template <typename T>
    Vector2<T>::Vector2() : x(0), y(0) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T>::Vector2(U x_, U y_) : x(x_), y(y_) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T>::Vector2(const Vector2<U>& other) : x(other.x), y(other.y) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T>::Vector2(Vector2<U>&& other) : x(other.x), y(other.y) {}

    template <typename T>
    T Vector2<T>::magnitude() const noexcept {
        return std::sqrt(x*x + y*y);
    }
    template <typename T>
    T Vector2<T>::sqr_magnitude() const noexcept {
        return x*x + y*y;
    }
    template <typename T>
    Vector2<T> Vector2<T>::normalized() const {
        return *this / magnitude();
    }
    template <typename T>
    Vector2<T>& Vector2<T>::normalize() {
        *this /= magnitude();
        return *this;
    }

    template <typename T>
    T Vector2<T>::dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }
    template <typename T>
    Vector2<T> Vector2<T>::zero() noexcept {
        return {0, 0, 0};
    }
    template <typename T>
    Vector2<T> Vector2<T>::one() noexcept {
        return {1, 1, 1};
    }
    template <typename T>
    Vector2<T> Vector2<T>::up() noexcept {
        return {0, 0, 1};
    }
    template <typename T>
    Vector2<T> Vector2<T>::down() noexcept {
        return {0, 0, -1};
    }
    template <typename T>
    Vector2<T> Vector2<T>::left() noexcept {
        return {0, 1, 0};
    }
    template <typename T>
    Vector2<T> Vector2<T>::right() noexcept {
        return {0, -1, 0};
    }
    template <typename T>
    Vector2<T> Vector2<T>::forward() noexcept {
        return {1, 0, 0};
    }
    template <typename T>
    Vector2<T> Vector2<T>::backward() noexcept {
        return {-1, 0, 1};
    }

    template <typename T>
    Vector2<T> Vector2<T>::operator+(const Vector2& other) const noexcept {
        return {x + other.x, y + other.y};
    }
    template <typename T>
    Vector2<T> Vector2<T>::operator-(const Vector2& other) const noexcept {
        return {x - other.x, y - other.y};
    }
    template <typename T>
    Vector2<T> Vector2<T>::operator-() const noexcept {
        return {-x, -y};
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T> Vector2<T>::operator*(U scalar) const noexcept {
        return {x * scalar, y * scalar};
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T> Vector2<T>::operator/(U scalar) const noexcept {
        return {x / scalar, y / scalar};
    }
    template <typename T>
    Vector2<T>& Vector2<T>::operator+=(const Vector2& other) noexcept {
        *this = *this + other;
        return *this;
    }
    template <typename T>
    Vector2<T>& Vector2<T>::operator-=(const Vector2& other) noexcept {
        *this = *this - other;
        return *this;
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T>& Vector2<T>::operator*=(U scalar) noexcept {
        *this = *this * scalar;
        return *this;
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector2<T>& Vector2<T>::operator/=(U scalar) noexcept {
        *this = *this / scalar;
        return *this;
    }

    template <typename T>
    bool Vector2<T>::operator==(const Vector2& other) const noexcept {
        return x == other.x && y == other.y;
    }

    template <typename T>
    T& Vector2<T>::operator[](std::size_t i) {
        switch(i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
            default:
                throw std::out_of_range("Index of Vector2 out of bounds!");
        }
    }
    template <typename T>
    T Vector2<T>::operator[](std::size_t i) const {
        switch(i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
            default:
                throw std::out_of_range("Index of Vector2 out of bounds!");
        }
    }

} // namespace Ferrite::Math::Vector
