#pragma once

Renderer::~Renderer() {
    flags.shouldContinue = false;
    std::free(results);
    
    std::free(buffer.data);
    std::free(buffer.depth);

    current = nullptr;
}

Renderer::Renderer(std::size_t width, std::size_t height, const Settings& settings, const Terraformer::Settings& terrainSettings) : 
	window(L"Renderer", width, height),
    terrain(terrainSettings, ((std::size_t)rand() << 32) % rand()),
    settings(settings)
{
    if (current) return;
    current = this;
    
    camera.Position = DBL::Vector3<float>{ 0.0f, 10.0f, 0.0f };

    processes = {
        Update([this]() { return !(frame % this->settings.ChunkUpdate); }, [&]() -> bool {

            const std::size_t size = 2 * this->settings.RenderDistance + 1;
            if(!results) results = (bool*)std::malloc(size * size);
            if (!results) return false;

            std::memset(results, 0, size * size);

            DBL::Vector2<std::int64_t> coordinates = terrain.ToChunk(camera.Position);
            if (meshes.size()) {
                for (std::size_t index = meshes.size() - 1; index; index--) {
                    if (!(meshes[index].Extra & Mesh3D<>::Chunk)) continue;

                    DBL::Vector2<std::int64_t> chunk = terrain.ToChunk(meshes[index].Center);
                    if (DBL::Distance(coordinates, chunk) > (float)this->settings.RenderDistance) {
                        meshes.erase(meshes.begin() + index);
                        continue;
                    }

                    chunk -= coordinates;
                    results[(chunk.Y + this->settings.RenderDistance) * size + (chunk.X + this->settings.RenderDistance)] = true;
                }
            }

            bool change = false;
            for (std::size_t Z = 0; Z < size; Z++) {
                for (std::size_t X = 0; X < size; X++) {
                    if (results[Z * size + X]) continue;
                    
                    change = true;
                    Mesh3D<> mesh = {};

                    DBL::Vector2<std::int64_t> offset = DBL::Vector2<std::int64_t>{ X, Z }
                        - DBL::Vector2<std::int64_t>((std::int64_t)this->settings.RenderDistance)
                        + coordinates;

                    terrain.Generate(offset, mesh);
                    mesh.Center = terrain.ToPosition(offset);

                    meshes.push_back(mesh);
                }
            }

            return change;
        }),
    };

    controls = {
        Keybind('W', [this]() { camera.Position += camera.Movements[2] * this->settings.Speed; return true; }),
        Keybind('A', [this]() { camera.Position += camera.Movements[0] * this->settings.Speed; return true; }),
        Keybind('S', [this]() { camera.Position -= camera.Movements[2] * this->settings.Speed; return true; }),
        Keybind('D', [this]() { camera.Position -= camera.Movements[0] * this->settings.Speed; return true; }),
        Keybind(VK_SPACE, [this]() { camera.Position += camera.Movements[1] * this->settings.Speed; return true; }),
        Keybind(VK_SHIFT, [this]() { camera.Position -= camera.Movements[1] * this->settings.Speed; return true; })
    };

    buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
    buffer.info.bmiHeader.biPlanes = 1;
    buffer.info.bmiHeader.biBitCount = 32;
    buffer.info.bmiHeader.biCompression = BI_RGB;

    Resize();
    window.Callback = &Callback;    // Only accept callbacks AFTER setup completes
};

void Renderer::Dispatch() {
    Main();
}

constexpr bool Renderer::IsActive() const {
    return flags.shouldContinue;
}

void Renderer::Main() {
    flags.shouldRender = flags.shouldContinue = true;

    std::wstring string(400, L'\0');
    POLYTEXTW text[4] = { 0 };
    for (std::size_t index = 0; index < sizeof(text) / sizeof(POLYTEXTW); index++) {
        text[index].y = index * 24;
        text[index].lpstr = string.data() + (index * 100);
        text[index].uiFlags = text[index].n = 0;
    }
    

    std::chrono::milliseconds elapsed(1);
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    for (; flags.shouldContinue; frame++) {
        start = std::chrono::high_resolution_clock::now();
        window.Update();

        for (Update& update : processes) if (update.first()) updates.world |= update.second();
        for (Keybind& keybind : controls) if (::GetKeyState(keybind.first) & 0x8000) updates.view |= keybind.second();

        flags.shouldRender |= updates.world | updates.view;
        if (flags.shouldRender) {   // I'm a firm believer in minimal indentation, but to avoid a goto mess this is somewhat necessary
            std::memset(buffer.data, 0, window.Dimensions().Product() * sizeof(Gdiplus::Color));
            std::fill(buffer.depth, buffer.depth + window.Dimensions().Product(), std::numeric_limits<float>::max());

            //for (const Mesh3D<>& mesh : meshes) Render(mesh);
            std::for_each(std::execution::par_unseq, std::cbegin(meshes), std::cend(meshes), [this](const Mesh3D<>& mesh) { Render(mesh); });

            std::wstring replacement;
           
            if (updates.world) {
                replacement = std::to_wstring(meshes.size()) + L" meshes loaded";
                std::wmemcpy(string.data(), replacement.data(), replacement.size());
                text[0].n = replacement.size();
            }
            if (updates.view) {
                replacement = std::wstring(L"Position: { ") + std::to_wstring(camera.Position.X) 
                    + L", " + std::to_wstring(camera.Position.Y)
                    + L", " + std::to_wstring(camera.Position.Z) + L" }";

                std::wmemcpy(string.data() + 100, replacement.data(), replacement.size());
                text[1].n = replacement.size();

                replacement = std::wstring(L"Rotation: [") + std::to_wstring(camera.RotationXZ) + L", " + std::to_wstring(camera.RotationYZ) + L']';
                
                std::wmemcpy(string.data() + 200, replacement.data(), replacement.size());
                text[2].n = replacement.size();
            }

            replacement = std::to_wstring(1 + elapsed.count()) + L" mSPF";
            std::wmemcpy(string.data() + 300, replacement.data(), replacement.size());
            text[3].n = replacement.size();

            ::StretchDIBits(window.Context, 0, 0, window.Dimensions().X, window.Dimensions().Y, 0, 0, window.Dimensions().X, window.Dimensions().Y, buffer.data, &buffer.info, DIB_RGB_COLORS, SRCCOPY);
            ::PolyTextOutW(window.Context, text, sizeof(text) / sizeof(POLYTEXTW));
            flags.shouldRender = updates.view = updates.world = false;
        }
        else {
            // Extra work, for future
        }



        end = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (elapsed < settings.mSPF) std::this_thread::sleep_for(settings.mSPF - elapsed);
    }
}

void Renderer::Render(const Mesh3D<>& mesh) {
    if (!mesh.Visible) return;

    Triangle<2> projected;
    DBL::Vector3<float> rotated;

    for (const Triangle<3>& triangle : mesh.Faces) {
        std::size_t index = 0;

        // Projection
        for (; index < 3; index++) {
            rotated = mesh.Center + triangle.Points[index] - camera.Position;

            // Broken apart for optimization
            DBL::Rotate<DBL::Axis::Y>(rotated, camera.RotationXZ);
            DBL::Rotate<DBL::Axis::X>(rotated, camera.RotationYZ);

            if (rotated.Z < 0.0f) { index = 4; break; }

            // out.points[a] = { halfSWidth * projected.X / projected.Z + halfSWidth, halfSHeight * projected.Y / projected.Z + halfSHeight };
            projected.Points[index] = DBL::Vector2<float>{ window.Dimensions().X, window.Dimensions().Y } * 0.5f / rotated.Z;

            projected.Points[index].X *= rotated.X;
            projected.Points[index].Y *= rotated.Y;

            projected.Points[index].X /= (16.0f / 9.0f);        // Slight inaccuracy but I don't care

            projected.Points[index].X += window.Dimensions().X / 2.0f;
            projected.Points[index].Y += window.Dimensions().Y / 2.0f;

        }

        if (index > 3) continue;
        float distance = DBL::Distance(camera.Position, triangle.Points[0] + mesh.Center);
        
        // Additional calculation, clipping
        std::size_t top, middle, bottom;
        top = middle = bottom = 0;

        std::size_t count = 0;
        for (index = 0; index < 3; index++) {
            if (projected.Points[index].Y > projected.Points[top].Y) top = index;
            else if (projected.Points[index].Y < projected.Points[bottom].Y) bottom = index;

            if (projected.Points[index].X < 0.0f || projected.Points[index].X >= window.Dimensions().X
                || projected.Points[index].Y < 0.0f || projected.Points[index].Y >= window.Dimensions().Y)
                count++;
        }

        middle = 3 - top - bottom;
        if (count == 3 || middle < 0) continue;

        float slopes[3] = {
            (projected.Points[2].Y - projected.Points[1].Y) / (projected.Points[2].X - projected.Points[1].X),
            (projected.Points[2].Y - projected.Points[0].Y) / (projected.Points[2].X - projected.Points[0].X),
            (projected.Points[1].Y - projected.Points[0].Y) / (projected.Points[1].X - projected.Points[0].X)
        };

        float starts[3] = {
            projected.Points[2].Y - (slopes[0] * projected.Points[2].X),
            projected.Points[0].Y - (slopes[1] * projected.Points[0].X),
            projected.Points[1].Y - (slopes[2] * projected.Points[1].X)
        };


        // Rasterization
        DBL::Vector2<float> point = {};

        if (projected.Points[top].Y >= (float)window.Dimensions().Y) projected.Points[top].Y = (float)window.Dimensions().Y - 1.0f;
        for (; projected.Points[top].Y > std::max(projected.Points[middle].Y, 0.0f); projected.Points[top].Y--) {
            point.X = (std::isinf(slopes[bottom]) ? projected.Points[middle].X : (projected.Points[top].Y - starts[bottom]) / slopes[bottom]);
            point.Y = (std::isinf(slopes[middle]) ? projected.Points[bottom].X : (projected.Points[top].Y - starts[middle]) / slopes[middle]);

            if (point.X > point.Y) std::swap(point.X, point.Y);

            if (point.X < 0.0f) point.X = 0.0f;
            if (point.Y >= window.Dimensions().X) point.Y = (float)window.Dimensions().X - 1.0f;

            std::size_t index = (std::size_t)projected.Points[top].Y * window.Dimensions().X + (std::size_t)point.X;

            for (; point.X < point.Y; point.X++) {  // Possible condition discrepancy...?
                if (buffer.depth[++index] <= distance) continue;

                buffer.data[index] = triangle.Color;
                buffer.depth[index] = distance;
            }
        }

        if (projected.Points[middle].Y >= (float)window.Dimensions().Y) projected.Points[middle].Y = (float)window.Dimensions().Y - 1.0f;
        for (; projected.Points[middle].Y > std::max(projected.Points[bottom].Y, 0.0f); projected.Points[middle].Y--) {
            point.X = (std::isinf(slopes[middle]) ? projected.Points[top].X : (projected.Points[middle].Y - starts[middle]) / slopes[middle]);
            point.Y = (std::isinf(slopes[top]) ? projected.Points[middle].X : (projected.Points[middle].Y - starts[top]) / slopes[top]);

            if (point.X > point.Y) std::swap(point.X, point.Y);

            if (point.X < 0.0f) point.X = 0.0f;
            if (point.Y >= window.Dimensions().X) point.Y = (float)window.Dimensions().X - 1.0f;

            std::size_t index = (std::size_t)projected.Points[middle].Y * window.Dimensions().X + (std::size_t)point.X;

            for (; point.X < point.Y; point.X++) {  // Possible condition discrepancy...?
                if (buffer.depth[++index] <= distance) continue;

                buffer.data[index] = triangle.Color;
                buffer.depth[index] = distance;
            }
        }
    }

}

void Renderer::Resize() {
    if(buffer.data) std::free(buffer.data);
    if(buffer.depth) std::free(buffer.depth);

    buffer.data = (Gdiplus::Color*)std::malloc(window.Dimensions().Product() * sizeof(Gdiplus::Color));
    buffer.depth = (float*)std::malloc(window.Dimensions().Product() * sizeof(float));
    
    buffer.info.bmiHeader.biWidth = window.Dimensions().X;
    buffer.info.bmiHeader.biHeight = window.Dimensions().Y;

    ::SelectObject(window.Context, settings.Text.Font);
    ::SetTextColor(window.Context, settings.Text.Color.ToCOLORREF());

    current->client = { current->window.Client.right / 2, current->window.Client.bottom / 2 };
    flags.shouldRender = true;
}

bool Renderer::Callback(unsigned message, WPARAM wParam, LPARAM lParam) {
    if (!current) return false;

    switch (message) {
    case WM_DESTROY: {
        ::PostQuitMessage(0);
        current->flags.shouldRender = current->flags.shouldContinue = false;

        return false;
    }
    case WM_SIZE: {         // Size already taken in prior callback level
        current->Resize();
        return true;
    }
    case WM_KEYUP: {
        if (wParam != VK_CONTROL) return false;

        ::ShowCursor(current->flags.cursorLocked);
        current->flags.cursorLocked = !current->flags.cursorLocked;

        return true;
    }
    case WM_MOUSEMOVE: {
        if (!current->flags.cursorLocked) return false;
        
        static POINT difference = {}, screen = {};
        difference = { LOWORD(lParam) - current->client.x, current->client.y - HIWORD(lParam) };

        
        if (!difference.x && !difference.y) return true;
        current->camera.Rotate(difference.x * current->settings.Sensitivity, difference.y * current->settings.Sensitivity);

        screen = current->client;
        ::ClientToScreen(current->window.Handle, &screen);

        ::SetCursorPos(screen.x, screen.y);
        current->flags.shouldRender = current->updates.view = true;
        return true;
    }
    case WM_ERASEBKGND: {
        return true;
    }

    default:
        return false;

    }
}

Renderer* Renderer::current = nullptr;