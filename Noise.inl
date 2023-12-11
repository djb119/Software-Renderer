#pragma once

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
void Noise<dimensions, T, Random>::Initialize(std::size_t seed, DimensionType boundaries) {
	this->boundaries = boundaries;
	random.seed(seed);

	Generate(true);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
constexpr Noise<dimensions, T, Random>::Type Noise<dimensions, T, Random>::operator[](const VectorType& point) const {
	return Sample(point);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
void Noise<dimensions, T, Random>::Resize(DimensionType boundaries) {
	this->boundaries = boundaries;
	random.seed(seed);

	Generate(false);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
void Noise<dimensions, T, Random>::Regenerate(std::size_t seed) {
	if (seed) {
		this->seed = seed;
		random.seed(seed);
	}

	Generate(false);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
constexpr const std::size_t& Noise<dimensions, T, Random>::Seed() const {
	return seed;
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
constexpr const Noise<dimensions, T, Random>::DimensionType& Noise<dimensions, T, Random>::Dimensions() const {
	return boundaries;
}

#pragma region Perlin

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
Perlin<dimensions, T, Random>::~Perlin() {
	if(data) std::free(data);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
Perlin<dimensions, T, Random>::Perlin(const Perlin& perlin) {
	operator=(perlin);
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
Perlin<dimensions, T, Random>& Perlin<dimensions, T, Random>::operator=(const Perlin& perlin) {
	Noise<dimensions, T, Random>::operator=(perlin);

	if (data) std::free(data);
	data = (VectorType*)std::malloc(perlin.Boundaries().Area() * sizeof(VectorType));
	std::memcpy(data, perlin.data, perlin.Boundaries().Area() * sizeof(VectorType));
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
constexpr Perlin<dimensions, T, Random>::Type Perlin<dimensions, T, Random>::Sample(const VectorType& point) const {
	constexpr auto smooth = [](Type value) -> Type { return value * value * value * (value * (6 * value - 15) + 10); };
	static_assert(dimensions == 2, "Perlin::Sample(const Perlin::VectorType&) not yet implemented for dimensions other than 2.");

	for (std::size_t index = 0; index < dimensions; index++)
		if(this->Dimensions()[0] < point[0]) return static_cast<Type>(0);

	VectorType surrounding[DBL::Pow2<dimensions>::Value]{};

	std::fill_n(surrounding, DBL::Pow2<dimensions>::Value, DBL::Floor(point));
	
	surrounding[1].X++, surrounding[3].X++;
	surrounding[2].Y++, surrounding[3].Y++;

	/*for (std::size_t dimension = 0; dimension < dimensions; dimension++) {
		std::size_t skip = (std::size_t)1 << dimension;

		for (std::size_t index = skip; index < DBL::Pow2<dimensions>::Value; index += skip)
			for (std::size_t start = index; index < start + skip; index++)		// IT WORKS! (KILL ME)
				surrounding[index][dimension]++;
	}*/
	
	Type values[DBL::Pow2<dimensions>::Value] = {};

	for (std::size_t index = 0; index < 4; index++) {
		surrounding[index].X = std::clamp(surrounding[index].X, 0.0f, (Type)this->Dimensions()[0]);
		surrounding[index].Y = std::clamp(surrounding[index].Y, 0.0f, (Type)this->Dimensions()[1]);

		values[index] = data[(std::size_t)(surrounding[index].Y * this->Dimensions()[0] + surrounding[index].X)].Dot(point - surrounding[index]);
	}

	return DBL::Interpolate(DBL::Interpolate(values[0], values[1], smooth(point.X - std::floor(point.X))),
		DBL::Interpolate(values[2], values[3], smooth(point.X - std::floor(point.X))),
		smooth(point.Y - std::floor(point.Y)));

	/*for (std::size_t index = 0; index < DBL::Pow2<dimensions>::Value; index++) {
		for (std::size_t dimension = 0; dimension < dimensions; dimension++)
			if (surrounding[index][dimension] >= this->Dimensions()[dimension])
				surrounding[index][dimension] = (float)this->Dimensions()[dimension] - 1.0f;

		values[index] = surrounding[index].Dot(point - surrounding[index]);
	}

	for (std::size_t dimension = 0; dimension < dimensions; dimension++) {	// Nicely done
		std::size_t skip = (std::size_t)1 << dimension;
		
		for (std::size_t index = 0; index < DBL::Pow2<dimensions>::Value; index += skip)
			values[index] = DBL::Interpolate(values[index], values[index += skip], smooth(point[dimension] - std::floor(point[dimension])));
	}*/

	//return values[0];
}

template<std::size_t dimensions, std::floating_point T, std::uniform_random_bit_generator Random>
void Perlin<dimensions, T, Random>::Generate(bool first) {
	if (!first) std::free(data);

	const std::size_t count = this->Dimensions().Product();
	data = (VectorType*)std::malloc(count * sizeof(VectorType));
	if (!data) return;

	for (std::size_t index = 0; index < count; index++) {
		for (std::size_t dimension = 0; dimension < dimensions; dimension++)
			data[index][dimension] = static_cast<Type>(this->random());
		
		data[index].Normalize();
	}
}

#pragma endregion Perlin