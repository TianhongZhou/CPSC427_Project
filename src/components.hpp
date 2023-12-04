#pragma once
#include "common.hpp"
#include <vector>
#include <array>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

#include <SDL_mixer.h>

struct Maze
{

};

struct Zombie
{

};

struct Boss
{

};

struct Sniper
{

};

struct Door
{

};

struct Spikes
{

};

struct Ball
{
	float trail;
};

struct PlayerBullet
{

};

struct EnemyBullet
{

};

struct CombatLevel
{
    int counter = 0;
};

// Player component
struct Player
{
	std::array<Entity,3> healthBar;
	float maxHealth = 100.f;
	float currentHealth = 100.f;
};

// Entity is a part of Combat
struct Combat
{

};

//Particle
struct Particle
{
    float lifespan = 1.f;
};

// Main world room
// Entity is part of main world
struct MainWorld{};

// Turtles have a hard shell
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
	vec2 roomPositon;
	float roomScale;
	bool keyFrame;
};

// Swarm enemy
struct SwarmEnemy
{
//    float randomMoveTimer = 0.1f;
//    vec2 boundary;
//    bool keyFrame;
//    std::array<Entity,3> healthBar;
    float maxHealth = 1.f;
    float currentHealth = 1.f;
    float invincibilityTimer;
//    float attackTimer;
//    float attackCooldown;
//    int attackType;
};

// Swarm King
struct SwarmKing
{};

// PinBall enemy
struct PinBallEnemy
{
	float randomMoveTimer = 0.1f;
	vec2 boundary;
	bool keyFrame;
	std::array<Entity,3> healthBar;
	float maxHealth = 100.f;
	float currentHealth = 100.f;
	float invincibilityTimer;
	float attackTimer;
	float attackCooldown;
	int attackType;
};

// A timer that will highlight a room enemy
struct HighLightEnemy
{
	float timer_ms = 1000.f;
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
	std::vector<Entity> engagedEnemeis;
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

	bool moveable;

    bool hasGravity = true;

	float knockbackCoef;
};


struct playerFlipper {



};

struct HealthBar {
	vec2 initScale;
	vec2 initPos;
};


struct mousePos {
	vec2 pos;
};


struct Light {
	vec2 screenPosition;
	float haloRadius;
	vec3 lightColor;
	float haloSoftness;
	int priority;
};

struct PositionKeyFrame {
	std::vector<vec3> keyFrames;
	float timeIncrement;
	float timeAccumulator;
};


struct PinballPlayerStatus {
	float health;
	float invincibilityTimer;
	float highGravityTimer;
	float antiGravityTimer;
	float dashCooldown;
	float focusTimer;
	float tractorTimer;
	int comboCounter;
};

struct DamageToPlayer {
	float damage;
};

struct DamageToEnemy {
	float damage;
};

struct TemporaryProjectile {
	int hitsLeft;
	float timeLeft;
	bool bonusBall;
};

struct PinBall {
	float pinBallSize = 10.f;
	float pinBallDamage = 5.f;
	float maxPinBallSize = 60.f;
	float maxPinBallDamage = 50.f;
	int antiGravityCount = 0.0f;
	int tractorBeamCount = 0.0f;
};

struct DropBuff {
	// 0 - pinball size, 1 - pinball damage , 2- antiGravity, 3 - tractorBeam
	int id;
	float increaseValue;
};


struct soundForPhys {
	Mix_Chunk* enemy_death_sound;

	Mix_Chunk* enemy_hit_sound;

	Mix_Chunk* player_hit_sound;
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
	START = FISH + 1,
	TUTORIAL = START + 1,
	TURTLE = TUTORIAL + 1,
	PLAYER = TURTLE + 1,
	PLAYERATTACK = PLAYER + 1,
	GROUND = PLAYERATTACK + 1,
	PLAYERATTACKSPRITESHEET = GROUND + 1,
	PLAYERDEATHSPRITESHEET = PLAYERATTACKSPRITESHEET + 1,
	PLAYERWALKSPRITESHEET = PLAYERDEATHSPRITESHEET + 1,
	ENEMYATTACKSPRITESHEET = PLAYERWALKSPRITESHEET + 1,
	ENEMYWALKSPRITESHEET = ENEMYATTACKSPRITESHEET + 1,
	SHADOW = ENEMYWALKSPRITESHEET + 1,
	PLAYERBULLET = SHADOW + 1,
	ENEMYBULLET = PLAYERBULLET + 1,
	DROPBALLSIZE = ENEMYBULLET + 1,
	DROPBALLDAMAGE = DROPBALLSIZE + 1,
	DROPBEAM = DROPBALLDAMAGE + 1,
	DROPGRAVITY = DROPBEAM + 1,
	PINBALL = DROPGRAVITY + 1,
	PINBALLBACKGROUND = PINBALL + 1,
	FLIPPER = PINBALLBACKGROUND + 1,
	WALL = FLIPPER + 1,
	GROUNDNORMAL = WALL + 1,
	TEXTURE_COUNT = GROUNDNORMAL + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	SALMON = PEBBLE + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	POST = WATER + 1,
	NORMAL = POST + 1,
	EFFECT_COUNT = NORMAL + 1,
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	BALL = PEBBLE + 1,
	PINBALLENEMYBLOOD = BALL + 1,
	PINBALLENEMY = PINBALLENEMYBLOOD + 1,
	ENEMYWAVE = PINBALLENEMY + 1,
	ROOM = ENEMYWAVE + 1,
	ROAD = ROOM + 1,
	PLAYER = ROAD + 1,
	RECT = PLAYER + 1,
	OCT = RECT + 1,
	DEBUG_LINE = OCT + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
    SWARMENEMY = SCREEN_TRIANGLE + 1,
    GEOMETRY_COUNT = SWARMENEMY + 1,
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	TEXTURE_ASSET_ID used_normal = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	vec2 textureOffset = vec2(0.0, 0.0);
	vec2 translationOffest = vec2(0.0, 0.0);
};

struct SpriteSheet {
	TEXTURE_ASSET_ID next_sprite;
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
