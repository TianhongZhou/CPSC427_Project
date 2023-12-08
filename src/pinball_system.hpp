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
#include "swarm_system.hpp"

// forward declaration
// https://stackoverflow.com/questions/4757565/what-are-forward-declarations-in-c
class WorldSystem;

// Game logic associated with pinball combat system
class PinballSystem
{
public:
    PinballSystem();

    // starts the game
    void init(GLFWwindow* window, RenderSystem* renderer, WorldSystem* world);

    // Releases all associated resources
    ~PinballSystem();

    // Steps the game ahead by ms milliseconds
    bool step(float elapsed_ms);

    // Check for collisions
    void handle_collisions();

    // exit combat
    void exit_combat();


private:
    void start_base_level();
    void start_test_level();
    void start_level_1();
    void start_level_2();
    void start_level_3();

    // redirect inputs to pinball callback functions
    void redirect_inputs_pinball();

    // Input callback functions
    void on_key(int key, int, int action, int mod);
    void on_mouse_move(vec2 pos);
    void on_mouse_click(int button, int action, int mods);

    void stepEnemyAttack();

    // sets current level states to default and starts current level
    void restart();

    float particleSpawnTimer = 0.f;

    Entity* backgrounds;

    // OpenGL window handle
    GLFWwindow* window;

    RenderSystem* renderer;

    WorldSystem* world;

    SwarmSystem swarmSystem = SwarmSystem(renderer);

    // C++ random number generator
//    std::default_random_engine rng;
//    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    std::unordered_set<int> pressedKeys;

    // FIXME: moved to swarm_system
//    void spawn_swarm(vec2 boundary);
//
//    // boids algorithm
//    // https://eater.net/boids
//    // http://www.kfish.org/boids/pseudocode.html
//    void update_swarm_motion();
//
//    // boids try to fly towards the centre of mass of neighbouring boids
//    // coherence determines by how much
//    vec2 rule1(Entity b_j, float coherence);
//
//    // boids try to keep a small distance away from other boids
//    // separation determines when boids are too close
//    vec2 rule2(Entity b_j, float separation);
//
//    // boids try to match velocity with near boids
//    // alignment determines by how much
//    vec2 rule3(Entity b, float alignment);
//
//    // boids try to go to position of the swarm king
//    // strength is by how much
//    vec2 rule4(Entity swarmKing, Entity b_j, float strength);
};
