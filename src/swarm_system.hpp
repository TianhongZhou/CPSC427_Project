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

//    GLFWwindow* window;
//
    RenderSystem* renderer;

    // detect swarm collision with ball
    // due to the way the physics system was built it is hard to use with the swarm :(
    // instead we use simple collision detection

    bool collides(Entity b_j);

    void handle_collision(Entity b_j);

    // some of these numbers are inverted because of scuffed programing
    // higher does not necessarily mean more

    // boids try to fly towards the centre of mass of neighbouring boids
    // coherence determines by how much
    vec2 rule1(Entity b_j, float coherence);

    // boids try to keep a small distance away from other boids
    // separation determines when boids are too close
    vec2 rule2(Entity b_j, float separation);

    // boids try to match velocity with near boids
    // alignment determines by how much
    vec2 rule3(Entity b_j, float alignment);

    // boids try to go to position of the swarm king
    // strength is by how much
    vec2 rule4(Entity swarmKing, Entity b_j, float strength);

    // boids try to leave if they get too close to the swarm king
    // leader separation determines when they are too close
    vec2 rule5(Entity swarmKing, Entity b_j, float leader_separation);
};
