#pragma once
#include "common.hpp"
#include <vector>
#include <array>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{

};

// Entity is a part of Combat
struct Combat
{

};

// Main world room
struct Room
{
	std::array<Entity,3> enemies;
};

// Main world enemy
struct Enemy
{
	bool seePlayer = false;
	float randomMoveTimer = 0.1f;
	float haltTimer = 0.3f;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity& other_entity) { this->other_entity = other_entity; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float timer_ms = 3000.f;
};

// A timer associated when entering combat scene
struct EnterCombatTimer
{
    float timer_ms = 1000.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
	bool fromVertices(const std::vector<vec2>& inputVertices,
		std::vector<ColoredVertex>& out_vertices,
		std::vector<uint16_t>& out_vertex_indices,
		vec2& out_size);
};



struct Vertex_Phys {
	vec2 pos;
	vec2 oldPos;

	vec2 accel;

};

struct physObj;


struct Edge {
	int v1; //array indices for the vertex array
	int v2;

	

	float len;

};

struct physObj {

	Vertex_Phys Vertices[8];
	Edge Edges[13];


	vec2 center;

	int VertexCount;
	int EdgesCount;


};


struct playerFlipper {



};


struct mousePos {
	vec2 pos;
};





/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	FISH = 0,
	TURTLE = FISH + 1,
	PLAYER = TURTLE + 1,
	PLAYERATTACK = PLAYER + 1,
	GROUND = PLAYERATTACK + 1,
	PLAYERATTACKSPRITESHEET = GROUND + 1,
	PLAYERWALKSPRITESHEET = PLAYERATTACKSPRITESHEET + 1,
	TEXTURE_COUNT = PLAYERWALKSPRITESHEET + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	SALMON = PEBBLE + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	EFFECT_COUNT = WATER + 1,
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	BALL = PEBBLE + 1,
	PINBALLENEMY = BALL + 1,
	ENEMYWAVE = PINBALLENEMY + 1,
	ROOM = ENEMYWAVE + 1,
	ROAD = ROOM + 1,
	PLAYER = ROAD + 1,
	RECT = PLAYER + 1,
	OCT = RECT + 1,
	DEBUG_LINE = OCT + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

struct SpriteSheet {
	TEXTURE_ASSET_ID sprite;
	int currentFrame = 0;
	int totalFrames;
	float frameIncrement;
	float frameAccumulator;
	int spriteSheetWidth;
	int spriteSheetHeight;
	bool loop = false;
	TEXTURE_ASSET_ID origin;
	bool xFlip = false;
};