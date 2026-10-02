#include "App.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

App::App()
    : window(nullptr),
      renderer(nullptr),
      texture(nullptr),
      running(true),
      paused(false) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(std::string("Could not initialize SDL: ") + SDL_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    window = SDL_CreateWindow(
        "Stigmergic Pixels",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        960,
        540,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(std::string("Could not create the window: ") + SDL_GetError());
    }

    SDL_SetWindowMinimumSize(window, 640, 360);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("Could not create the renderer: ") + SDL_GetError());
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    configureStyle();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)) {
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Could not initialize the Dear ImGui SDL2 backend.");
    }

    if (!ImGui_ImplSDLRenderer2_Init(renderer)) {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Could not initialize the Dear ImGui renderer backend.");
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        Simulation::width,
        Simulation::height
    );

    if (texture == nullptr) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("Could not create the trail texture: ") + SDL_GetError());
    }
}

App::~App() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void App::run() {
    using clock = std::chrono::steady_clock;

    constexpr float fixedStep = 1.0f / 60.0f;
    float accumulator = 0.0f;
    auto previousTime = clock::now();

    while (running) {
        const auto currentTime = clock::now();
        const std::chrono::duration<float> elapsed = currentTime - previousTime;
        previousTime = currentTime;
        accumulator += std::min(elapsed.count(), 0.1f);

        processEvents();

        if (!paused) {
            while (accumulator >= fixedStep) {
                simulation.update();
                accumulator -= fixedStep;
            }
        } else {
            accumulator = 0.0f;
        }

        render();
    }
}

void App::processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && !ImGui::GetIO().WantCaptureKeyboard) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_SPACE:
                    paused = !paused;
                    SDL_SetWindowTitle(
                        window,
                        paused ? "Stigmergic Pixels - Paused" : "Stigmergic Pixels"
                    );
                    break;
                case SDLK_r:
                    simulation.reset();
                    break;
                default:
                    break;
            }
        }
    }
}

void App::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    renderControlPanel();

    const std::vector<std::uint32_t>& pixels = simulation.pixels();

    if (SDL_UpdateTexture(
            texture,
            nullptr,
            pixels.data(),
            Simulation::width * static_cast<int>(sizeof(std::uint32_t))) != 0) {
        throw std::runtime_error(std::string("Could not update the trail texture: ") + SDL_GetError());
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void App::renderControlPanel() {
    ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 322.0f), ImGuiCond_Always);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("SIMULATION CONTROLS", nullptr, flags);

    ImGui::Checkbox("ATTRACTION", &simulation.attraction());
    ImGui::Checkbox("REPULSION", &simulation.repulsion());

    ImGui::Separator();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("AGENTS");
    ImGui::SameLine(116.0f);

    int agentCount = simulation.agentCount();
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputInt("##agents", &agentCount, 100, 1000)) {
        simulation.setAgentCount(agentCount);
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("SPECIES");
    ImGui::SameLine(116.0f);

    int speciesCount = simulation.speciesCount();
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputInt("##species", &speciesCount, 1, 1)) {
        simulation.setSpeciesCount(speciesCount);
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("SPEED");
    ImGui::SameLine(116.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat(
        "##speed",
        &simulation.speed(),
        0.1f,
        5.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("DIFFUSION");
    ImGui::SameLine(116.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat(
        "##diffusion",
        &simulation.diffusionRate(),
        0.0f,
        1.0f,
        "%.3f",
        ImGuiSliderFlags_AlwaysClamp
    );

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("EVAPORATION");
    ImGui::SameLine(116.0f);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat(
        "##evaporation",
        &simulation.evaporationRate(),
        0.0f,
        0.20f,
        "%.4f",
        ImGuiSliderFlags_AlwaysClamp
    );

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    if (ImGui::Button("RESET", ImVec2(-1.0f, 30.0f))) {
        simulation.reset();
    }

    ImGui::End();
}

void App::configureStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(12.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.GrabMinSize = 12.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    const ImVec4 background(0.047f, 0.047f, 0.047f, 0.96f);
    const ImVec4 foreground(0.80f, 0.80f, 0.80f, 1.0f);
    const ImVec4 dimmed(0.36f, 0.36f, 0.36f, 1.0f);

    colors[ImGuiCol_WindowBg] = background;
    colors[ImGuiCol_TitleBg] = background;
    colors[ImGuiCol_TitleBgActive] = background;
    colors[ImGuiCol_TitleBgCollapsed] = background;
    colors[ImGuiCol_Text] = foreground;
    colors[ImGuiCol_TextDisabled] = dimmed;
    colors[ImGuiCol_Border] = foreground;
    colors[ImGuiCol_FrameBg] = background;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = background;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
    colors[ImGuiCol_CheckMark] = foreground;
    colors[ImGuiCol_Separator] = foreground;
}
