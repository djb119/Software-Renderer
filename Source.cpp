#define __DBL_ExplicitWinMain
#define __DBL_ManualSIMD false
#include <DBL/Graphics>

#include "Mesh.hpp"
#include "Noise.hpp"
#include "Camera.hpp"
#include "Terraformer.hpp"
#include "Renderer.hpp"

int wWinMain(HINSTANCE hInstance, HINSTANCE previous, wchar_t* command, int show) {
	Renderer::Settings render;
	render.RenderDistance = 4;
	render.Speed = 20.0f;

	Terraformer::Settings terrain;
	terrain.HeightColor = true;
	terrain.Shadowing = false;
	terrain.Resolution = 1.0f;
	terrain.Size = 20;
	terrain.Stretch = 50.0f;

	Renderer renderer(500, 500, render, terrain);
	renderer.Dispatch();

	return 0;
}