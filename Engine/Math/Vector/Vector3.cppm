module;

#include <stdexcept>
#include <type_traits>
#include <cmath>

export module Ferrite.Math.Vector.Vector3;

namespace Ferrite::Math::Vector {

    export template <typename T> struct Vector3 {
        T x, y, z;

        Vector3();
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3(U x_, U y_, U z_);
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3(const Vector3<U>&);
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3(Vector3<U>&&);

        T magnitude() const noexcept;
        T sqr_magnitude() const noexcept;
        Vector3 normalized() const;
        Vector3& normalize();

        static T dot(const Vector3& a, const Vector3& b);
        static Vector3 cross(const Vector3& a, const Vector3& b);

        static Vector3 zero() noexcept;
        static Vector3 one() noexcept;
        static Vector3 up() noexcept;
        static Vector3 down() noexcept;
        static Vector3 left() noexcept;
        static Vector3 right() noexcept;
        static Vector3 forward() noexcept;
        static Vector3 backward() noexcept;

        Vector3 operator+(const Vector3& other) const noexcept;
        Vector3 operator-(const Vector3& other) const noexcept;
        Vector3 operator-() const noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3 operator*(U scalar) const noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3 operator/(U scalar) const noexcept;

        Vector3& operator+=(const Vector3& other) noexcept;
        Vector3& operator-=(const Vector3& other) noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3& operator*=(U scalar) noexcept;
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vector3& operator/=(U scalar) noexcept;

        bool operator==(const Vector3& other) const noexcept;

        T& operator[](std::size_t i);
        T operator[](std::size_t i) const;
    };

    template <typename T, typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T> operator*(U scalar, const Vector3<T>& vec) noexcept {
        return vec * scalar;
    }

    template <typename T>
    Vector3<T>::Vector3() : x(0), y(0), z(0) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T>::Vector3(U x_, U y_, U z_) : x(x_), y(y_), z(z_) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T>::Vector3(const Vector3<U>& other) : x(other.x), y(other.y), z(other.z) {}
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T>::Vector3(Vector3<U>&& other) : x(other.x), y(other.y), z(other.z) {}

    template <typename T>
    T Vector3<T>::magnitude() const noexcept {
        return std::sqrt(x*x + y*y + z*z);
    }
    template <typename T>
    T Vector3<T>::sqr_magnitude() const noexcept {
        return x*x + y*y + z*z;
    }
    template <typename T>
    Vector3<T> Vector3<T>::normalized() const {
        return *this / magnitude();
    }
    template <typename T>
    Vector3<T>& Vector3<T>::normalize() {
        *this /= magnitude();
        return *this;
    }

    template <typename T>
    T Vector3<T>::dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    template <typename T>
    Vector3<T> Vector3<T>::cross(const Vector3& a, const Vector3& b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }
    template <typename T>
    Vector3<T> Vector3<T>::zero() noexcept {
        return {0, 0, 0};
    }
    template <typename T>
    Vector3<T> Vector3<T>::one() noexcept {
        return {1, 1, 1};
    }
    template <typename T>
    Vector3<T> Vector3<T>::up() noexcept {
        return {0, 0, 1};
    }
    template <typename T>
    Vector3<T> Vector3<T>::down() noexcept {
        return {0, 0, -1};
    }
    template <typename T>
    Vector3<T> Vector3<T>::left() noexcept {
        return {0, 1, 0};
    }
    template <typename T>
    Vector3<T> Vector3<T>::right() noexcept {
        return {0, -1, 0};
    }
    template <typename T>
    Vector3<T> Vector3<T>::forward() noexcept {
        return {1, 0, 0};
    }
    template <typename T>
    Vector3<T> Vector3<T>::backward() noexcept {
        return {-1, 0, 1};
    }

    template <typename T>
    Vector3<T> Vector3<T>::operator+(const Vector3& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }
    template <typename T>
    Vector3<T> Vector3<T>::operator-(const Vector3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }
    template <typename T>
    Vector3<T> Vector3<T>::operator-() const noexcept {
        return {-x, -y, -z};
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T> Vector3<T>::operator*(U scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T> Vector3<T>::operator/(U scalar) const noexcept {
        return {x / scalar, y / scalar, z / scalar};
    }
    template <typename T>
    Vector3<T>& Vector3<T>::operator+=(const Vector3& other) noexcept {
        *this = *this + other;
        return *this;
    }
    template <typename T>
    Vector3<T>& Vector3<T>::operator-=(const Vector3& other) noexcept {
        *this = *this - other;
        return *this;
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T>& Vector3<T>::operator*=(U scalar) noexcept {
        *this = *this * scalar;
        return *this;
    }
    template <typename T>
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vector3<T>& Vector3<T>::operator/=(U scalar) noexcept {
        *this = *this / scalar;
        return *this;
    }

    template <typename T>
    bool Vector3<T>::operator==(const Vector3& other) const noexcept {
        return x == other.x && y == other.y && z == other.y;
    }

    template <typename T>
    T& Vector3<T>::operator[](std::size_t i) {
        switch(i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
            case 2:
                return z;
                break;
            default:
                throw std::out_of_range("Index of Vector3 out of bounds!");
        }
    }
    template <typename T>
    T Vector3<T>::operator[](std::size_t i) const {
        switch(i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
            case 2:
                return z;
                break;
            default:
                throw std::out_of_range("Index of Vector3 out of bounds!");
        }
    }

} // namespace Ferrite::Math::Vector
