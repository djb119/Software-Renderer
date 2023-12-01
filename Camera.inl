#pragma once

template<DBL::Numeric T>
inline void Camera<T>::Rotate(T rXZ, T rYZ) {
    RotationXZ += rXZ;
    RotationYZ += rYZ;

    Movements[0] = { std::cos(RotationXZ), 0.0f, std::sin(RotationXZ) };
    Movements[2] = { Movements[0].Z, 0.0f, Movements[0].X };
};