#pragma once

#include <random>

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
class Noise {
public:

	using Type = T;

	using VectorType = DBL::Vector<dimensions, Type>;
	using DimensionType = DBL::Vector<dimensions, std::size_t>;

	static_assert(dimensions, "Noise must have at least 1 dimension.");


	Noise() = default;
	~Noise() = default;

	Noise(Noise&&) = delete;
	Noise(const Noise&) = default;

	Noise& operator=(Noise&&) = delete;
	Noise& operator=(const Noise&) = default;


	constexpr Type operator[](const VectorType& point) const;
	constexpr virtual Type Sample(const VectorType& point) const = 0;

	void Resize(DimensionType dimensions);
	void Regenerate(std::size_t seed = 0);

	constexpr const std::size_t& Seed() const;
	constexpr const DimensionType& Dimensions() const;


	inline void Initialize(std::size_t seed, DimensionType boundaries);

protected:

	Random random;


	virtual void Generate(bool first) = 0;

private:

	std::size_t seed = 0;
	DimensionType boundaries;

};


template<std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
using Noise2D = Noise<2, T, Random>;

template<std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
using Noise3D = Noise<3, T, Random>;


template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
class Perlin : public Noise<dimensions, T, Random> {
public:

	using Type = Noise<dimensions, T, Random>::Type;

	using VectorType = Noise<dimensions, T, Random>::VectorType;
	using DimensionType = Noise<dimensions, T, Random>::DimensionType;


	Perlin() = default;
	~Perlin();

	Perlin(const Perlin& perlin);

	Perlin& operator=(const Perlin& perlin);

	constexpr Type Sample(const VectorType& point) const override;

protected:

	void Generate(bool first) override;

private:

	VectorType* data = nullptr;	// Recursive pointer levels not required

};


template<std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
using Perlin2D = Perlin<2, T, Random>;

template<std::floating_point T, std::uniform_random_bit_generator Random = std::default_random_engine>
using Perlin3D = Perlin<3, T, Random>;


#include "Noise.inl"