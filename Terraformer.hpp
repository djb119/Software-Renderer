#pragma once

#include <cstdarg>

class Terraformer final {
public:

	struct Settings;

	Terraformer() = delete;
	~Terraformer();

	Terraformer(const Settings& settings, std::size_t seed, ...);

	Terraformer(Terraformer&&) = delete;
	Terraformer(const Terraformer&) = delete;

	Terraformer& operator=(Terraformer&&) = delete;
	Terraformer& operator=(const Terraformer&) = delete;


	static constexpr std::size_t Biomes = 5;


	void Generate(const DBL::Vector2<std::int64_t>& coordinates, Mesh3D<>& mesh);

	inline DBL::Vector2<std::int64_t> ToChunk(const DBL::Vector3<float>& position) const;
	inline DBL::Vector3<float> ToPosition(const DBL::Vector2<std::int64_t>& chunk) const;

	struct Biome {
	
		std::wstring Name;
		Gdiplus::Color Color;

		std::function<float(const DBL::Vector2<float>&, Noise2D<float>*)> Applies = nullptr;
		std::function<void(const DBL::Vector2<float>&, Noise2D<float>*, float&)> Place = nullptr;

		Noise2D<float>* Noise = nullptr;

	};


	struct Settings {
		float Base = -5.0f, Resolution = 1.0f;
		float Stretch = 10.0f, Density = 1.0f;

		float Blend = 3.0f;

		std::int64_t Chunks = 3000, Size = 16;
		bool HeightColor = false, Shadowing = false;

		Gdiplus::Color Color = Gdiplus::Color::LawnGreen;

	} const Configuration;

private:

	Perlin2D<float> main;

	std::vector<Biome> biomes;

};


#include "Terraformer.inl"