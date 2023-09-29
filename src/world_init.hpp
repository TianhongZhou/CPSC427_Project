#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <numeric>

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;

// the ball
Entity createBall(RenderSystem* renderer, vec2 pos);
// the polygon
Entity createPolygonByVertex(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id);
// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 pos);
// the enemy wave
Entity createEnemyWave(RenderSystem* renderer, vec2 pos);
// the player
Entity createSalmon(RenderSystem* renderer, vec2 pos);
// the prey
Entity createFish(RenderSystem* renderer, vec2 position);
// the enemy
Entity createTurtle(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a pebble
Entity createPebble(vec2 pos, vec2 size);


