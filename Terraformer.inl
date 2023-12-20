#pragma once

#undef max

Terraformer::~Terraformer() {
	for (Biome& biome : biomes) delete biome.Noise;
}

Terraformer::Terraformer(const Settings& settings, std::size_t seed, ...) :
	Configuration(settings)
{
	main.Initialize(seed, DBL::Vector2<std::size_t>((std::size_t)settings.Chunks));

	std::va_list list;
	va_start(list, seed);

	// TODO: Create biomes, sort by decreasing importance
	va_end(list);
}

void Terraformer::Generate(const DBL::Vector2<std::int64_t>& coordinates, Mesh3D<>& mesh) {
	if (coordinates.X > Configuration.Chunks || coordinates.X < -Configuration.Chunks || coordinates.Y > Configuration.Chunks || coordinates.Y < -Configuration.Chunks) return;
	const float increment = 1.0f / std::max(1.0f / Configuration.Resolution, 1.0f / (float)Configuration.Size);

	DBL::Vector2<float> points[4] = {}, queries[4] = {};
	Triangle<3> triangles[2] = {};

	float heights[4] = {};

	for (float Z = 0.0f; Z < Configuration.Size; Z += increment) {
		for (float X = 0.0f; X < Configuration.Size; X += increment) {

			points[0] = { X, Z };
			points[1] = { X + increment, Z };
			points[2] = { X, Z + increment };
			points[3] = { X + increment, Z + increment };

			for (std::size_t index = 0; index < 4; index++) {
				queries[index] = DBL::Vector2<float>{ coordinates.X, coordinates.Y } + (points[index] / (float)Configuration.Size);
				queries[index] = (queries[index] / DBL::Vector2<float>{ (float)Configuration.Chunks, (float)-Configuration.Chunks }) + DBL::Vector2<float>(1.0f);
				queries[index] /= Configuration.Density;

				heights[index] = main.Sample(queries[index] * DBL::Vector2<float>{ main.Dimensions().X, main.Dimensions().Y } * 0.5f) * Configuration.Stretch + Configuration.Base;
			}
			
			Biome* chosen = nullptr;
			for (Biome& biome : biomes) {
				if (!biome.Applies(queries[0], biome.Noise)) continue;

				chosen = &biome;
				break;
			}

			if (chosen) {
				// Unrolled in a stab at efficiency, not sure
				chosen->Place(queries[0], chosen->Noise, heights[0]);
				chosen->Place(queries[1], chosen->Noise, heights[1]);
				chosen->Place(queries[2], chosen->Noise, heights[2]);
				chosen->Place(queries[3], chosen->Noise, heights[3]);

				triangles[0].Color.Base = triangles[1].Color.Base = chosen->Color;
			}
			else triangles[0].Color.Base = triangles[1].Color.Base = Configuration.Color;


			for (std::size_t index = 0; index < 2; index++)
				for (std::size_t point = 0; point < 3; point++)
					triangles[index].Points[point] = { points[index + point].X , heights[index + point], points[index + point].Y };


			mesh.Faces.push_back(triangles[0]);
			mesh.Faces.push_back(triangles[1]);
		}
	}

	mesh.Extra = Mesh3D<>::Flags::Chunk;
	mesh.Visible = true;
}

DBL::Vector2<std::int64_t> Terraformer::ToChunk(const DBL::Vector3<float>& position) const {
	return DBL::Vector2<std::int64_t>{ (std::int64_t)std::floor(position.X / Configuration.Size),
		(std::int64_t)std::floor(position.Z / Configuration.Size) };
}

DBL::Vector3<float> Terraformer::ToPosition(const DBL::Vector2<std::int64_t>& chunk) const {
	return DBL::Vector3<float>{ chunk.X * Configuration.Size, 0.0f, chunk.Y * Configuration.Size };
}