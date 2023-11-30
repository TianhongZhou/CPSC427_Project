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
Entity createBall(RenderSystem* renderer, vec2 pos, float size, float trail);
// the shadow
Entity createShadow(RenderSystem* renderer, vec2 pos);
// the walls
Entity createPinballWall(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id);
// the flipper
Entity createPinballFlipper(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id);
// the pin ball enemy blood
Entity createHealth(RenderSystem* renderer, vec2 pos, bool combat);
// swarm enemies
Entity createSwarmEnemy(RenderSystem* renderer, vec2 pos);
// the pin ball enemy
Entity createPinBallEnemy(RenderSystem *renderer, vec2 pos, vec2 boundary, float xScale, int attackType, float attackCd,
                          float yScale);
// the room
Entity createRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window, int room_num);
// the pinball room
Entity createPinballRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos, float currentHealth);
// the room enemy
Entity createRoomEnemy(RenderSystem* renderer, vec2 pos, vec2 roomPosition, float roomScale, bool keyFrame);
// the player
Entity createSalmon(RenderSystem* renderer, vec2 pos);
// the prey
Entity createFish(RenderSystem* renderer, vec2 position);
// the enemy
Entity createTurtle(RenderSystem* renderer, vec2 position);
// a pebble
Entity createDoor(vec2 pos, vec2 size);

Entity createDoor(vec2 pos, vec2 size);

Entity createSpikes(vec2 pos, vec2 size);

Entity createStartingRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window);
Entity createRoom1(RenderSystem* renderer, vec2 pos);
Entity createRoom2(RenderSystem* renderer, vec2 pos);
Entity createRoom3(RenderSystem* renderer, vec2 pos);
Entity createEmptyRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window);

Entity createDropBuff(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID id);

Entity createParticle(RenderSystem* renderer, vec2 pos, float size, vec2 vel, vec3 color, float lifespan);

// player and enemy bullets
Entity createPlayerBullet(vec2 pos, vec2 size);
Entity createEnemyBullet(vec2 pos, vec2 size);

void createNewRectangleTiedToEntity(Entity e, float w, float h, vec2 centerPos, bool moveable, float knockbackCoef);


