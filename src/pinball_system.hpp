#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <unordered_set>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

class PinballSystem
{
public:
    PinballSystem();

    // starts the game
    void init(GLFWwindow* window, RenderSystem* renderer);

    // Releases all associated resources
    ~PinballSystem();

    // Steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // Check for collisions
    void handle_collisions();

    // Should the game be over ?
    bool is_over()const;

    // exit combat
    void exit_combat();


private:
    // redirect inputs to pinball callback functions
    void redirect_inputs_pinball();
    // Input callback functions
    void on_key(int key, int, int action, int mod);
    void on_mouse_move(vec2 pos);
    void on_mouse_click(int button, int action, int mods);

    // restart level
    void restart_game();

    // initialize combat
    void init_combat();

    // OpenGL window handle
    GLFWwindow* window;

    RenderSystem* renderer;

    // C++ random number generator
//    std::default_random_engine rng;
//    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    std::unordered_set<int> pressedKeys;
};
