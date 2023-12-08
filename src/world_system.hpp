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
#include "pinball_system.hpp"

extern float Enter_combat_timer;
extern int GameSceneState;
extern int InitCombat;
extern int MonitorWidth;
extern int MonitorHeight;
extern float MonitorScreenRatio;
extern int offsetX;
extern int offsetY;
extern int scaledWidth;
extern int scaledHeight;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step_world(float elapsed_ms);

	// Check for collisions
	void handle_collisions_world();

	// Should the game be over ?
	bool is_over()const;

	// initialize combat
	void init_combat(PinballSystem pinballSystem);

    // redirect keyboard/mouse inputs back into world callback functions
    void redirect_inputs_world();

    // restart level
    void restart_game();

	void save_game(const std::string& filename);
	void load_game(const std::string& filename);


	Mix_Chunk* enemy_death_sound;
	Mix_Chunk* dash_sound;
	Mix_Chunk* enemy_hit_sound;
	Mix_Chunk* flipper_sound;
	Mix_Chunk* player_hit_sound;


private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mods);

	void enter_next_room();
	void spawn_room_enemies(float elapsed_ms_since_last_update);
	void check_room_boundary();
	void save_player_last_direction();
	
	void DropBuffAdd(DropBuff& drop);
	void GenerateDropBuff(Entity entity);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	int static const MAX_ROOM_NUM = 9;
//	int curr_room = 0;
    Entity curr_rooom;

	RenderSystem* renderer;
	float current_speed;
	float generate_enemy_timer = 0.f;
	float spike_damage_timer = 0.f;
	float playerHealth = 100.f;
	Entity player_salmon;
	Entity player;
	std::array<Entity,MAX_ROOM_NUM> rooms;

	float last_angle = 0.f;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;

	Mix_Chunk* player_attack_sound;



	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	std::unordered_set<int> pressedKeys;

};
