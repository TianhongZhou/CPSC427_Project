#pragma once

// internal
#include "common.hpp"

#include "render_system.hpp"

// logic associated with the swarm enemy type
class SwarmSystem
{
public:
    SwarmSystem(RenderSystem* renderer_arg);

    ~SwarmSystem();

    // creates the swarm enemy
    void spawn_swarm(vec2 boundary);

    // boids algorithm
    // https://eater.net/boids
    // http://www.kfish.org/boids/pseudocode.html
    void update_swarm_motion();

private:

//    // OpenGL window handle
//    GLFWwindow* window;
//
    RenderSystem* renderer;

    // detect swarm collision with ball
    void detect_collision();

    void handle_collision();

    // boids try to fly towards the centre of mass of neighbouring boids
    // coherence determines by how much
    vec2 rule1(Entity b_j, float coherence);

    // boids try to keep a small distance away from other boids
    // separation determines when boids are too close
    vec2 rule2(Entity b_j, float separation);

    // boids try to match velocity with near boids
    // alignment determines by how much
    vec2 rule3(Entity b, float alignment);

    // boids try to go to position of the swarm king
    // strength is by how much
    // FIXME: there might be a bug in this as the boids seem to accelerate with time
    vec2 rule4(Entity swarmKing, Entity b_j, float strength);
};
