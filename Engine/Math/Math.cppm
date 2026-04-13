export module Ferrite.Math;

export import Ferrite.Math.Vector;

#if defined(FERRITE_USE_DOUBLE)
namespace Ferrite::Math {
    export using Vector3 = Ferrite::Math::Vector::Vector3<double>;
    export using Vector2 = Ferrite::Math::Vector::Vector2<double>;
}
#else
namespace Ferrite::Math {
    export using Vector3 = Ferrite::Math::Vector::Vector3<float>;
    export using Vector2 = Ferrite::Math::Vector::Vector2<float>;
}
#endif
