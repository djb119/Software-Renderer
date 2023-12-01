#pragma once

template<DBL::Numeric T = float>
struct Camera {
public:

    Camera() = default;
    ~Camera() = default;

    Camera(Camera&&) = default;
    Camera(const Camera&) = default;
    
    Camera& operator=(Camera&&) = default;
    Camera& operator=(const Camera&) = default;


    DBL::Vector3<T> Position = {};
    T RotationXZ = 0, RotationYZ = 0;

    DBL::Vector3<T> Movements[3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };


    inline void Rotate(T rXZ, T rYZ);

};

#include "Camera.inl"