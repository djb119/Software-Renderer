#pragma once

#include <thread>
#include <chrono>
#include <fstream>
#include <execution>
#include <windowsx.h>
#include <commdlg.h>

using DBL::Graphics::Window;

class Renderer final {
public:

	friend class Terraformer;

	using Update = std::pair<std::function<bool()>, std::function<bool()>>;
	using Keybind = std::pair<unsigned, std::function<bool()>>;

	struct Settings;


	~Renderer();

	Renderer(std::size_t width, std::size_t height, const Settings& settings, const Terraformer::Settings& terrainSettings);

	Renderer(Renderer&&) = delete;
	Renderer(const Renderer&) = delete;

	Renderer& operator=(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;


	struct Settings {
	public:

		float Sensitivity = 0.001f, Speed = 0.10f;	// TODO: Normalize with frame times
		std::size_t ChunkUpdate = 125;

		std::size_t RenderDistance = 10;
		std::chrono::milliseconds mSPF = std::chrono::milliseconds(1000 / 60);
		
		bool ColorByHeight = false, Shading = false, UseLighting = false;

		struct {
			HFONT Font = nullptr;
			Gdiplus::Color Color = {};

		} Text;

	};


	void Dispatch();

	constexpr bool IsActive() const;

private:

	Settings settings;


	struct {
		bool shouldContinue = false,
			shouldRender = false;

		bool cursorLocked = false;

		bool modelMode = false;

	} flags;

	
	struct {
		bool world = false,
			view = false;

	} updates;
	

	std::jthread thread;
	Window window;
	
	POINT client = {};

	Camera<> camera;
	std::vector<Mesh3D<>> meshes;
	std::vector<Light> lights;

	
	std::size_t frame = 0;
	std::vector<Update> processes;
	std::vector<Keybind> controls;

	struct { 
		Gdiplus::Color* data = nullptr; 
		float* depth = nullptr;

		BITMAPINFO info = {}; 
	} buffer;
	

	Terraformer terrain;
	bool* results = nullptr;


	void Main();

	void Render(const Mesh3D<>& mesh);
	void ComputeColor(Mesh3D<>& mesh) const;

	void Resize();
	

	void LoadModel(const wchar_t* path, Mesh3D<>& mesh, Gdiplus::Color color) const;


	static bool Callback(unsigned message, WPARAM wParam, LPARAM lParam);

	static Renderer* current;

};

#include "Renderer.inl"