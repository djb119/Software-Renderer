#pragma once

template<std::size_t dimensions, DBL::Numeric T = float>
struct Triangle final {
public:
	
	DBL::Vector<dimensions, T> Points[3] = {};
	Gdiplus::Color Color;


	DBL::Vector3<T> Normal() const {
		DBL::Vector<dimensions, T> first = Points[2] - Points[1], second = Points[3] - Points[1];
		return DBL::Cross(first, second);
	}

};


template<std::size_t dimensions, DBL::Numeric T = float>
struct Mesh {
public:

	static_assert(dimensions > 1, "Mesh must be at least 2-dimensional.");
	enum Flags : std::size_t { Chunk = 1 };

	Mesh() = default;
	~Mesh() = default;

	Mesh(Mesh&& mesh) noexcept { operator=(std::forward<Mesh>(mesh)); }
	Mesh(const Mesh& mesh) { operator=(mesh); }

	Mesh& operator=(Mesh&& mesh) noexcept {
		std::swap(Center, mesh.Center);
		std::swap(Velocity, mesh.Velocity);

		std::swap(Faces, mesh.Faces);
		std::swap(Visible, mesh.Visible);

		std::swap(Extra, mesh.Extra);
		std::memcpy(Rotations, mesh.Rotations, (dimensions - 1) * sizeof(T));

		return *this;
	}
	Mesh& operator=(const Mesh& mesh) {
		Center = mesh.Center;
		Velocity = mesh.Velocity;

		Faces = mesh.Faces;
		Visible = mesh.Visible;

		Extra = mesh.Extra;
		std::memcpy(Rotations, mesh.Rotations, (dimensions - 1) * sizeof(T));

		return *this;
	}


	DBL::Vector<dimensions, T> Center = {}, Velocity = {};

	std::vector<Triangle<dimensions, T>> Faces;	// Centered locally around origin
	T Rotations[dimensions - 1] = {};

	bool Visible = true;

	std::size_t Extra = 0;

};


template<DBL::Numeric T = float>
using Polygon2D = Triangle<2, T>;

template<DBL::Numeric T = float>
using Polygon3D = Triangle<3, T>;

template<DBL::Numeric T = float>
using Mesh2D = Mesh<2, T>;

template<DBL::Numeric T = float>
using Mesh3D = Mesh<3, T>;