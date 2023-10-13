#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

extern int GameSceneState;
extern int InitCombat;


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
	bool step(float elapsed_ms);
	bool step_world(float elapsed_ms);

	// Check for collisions
	void handle_collisions();
	void handle_collisions_world();

	// Should the game be over ?
	bool is_over()const;

	// initialize combat
	void init_combat();

    // exit combat
    void exit_combat();

	//set main world stuff out of sight
	void main_world_out();

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mods);

	// restart level
	void restart_game();



	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	int static const MAX_ROOM_NUM = 9;
	int static const ENEM_NUM_PER_ROOM = 2;

	RenderSystem* renderer;
	float current_speed;
	float next_turtle_spawn;
	float next_fish_spawn;
	Entity player_salmon;
	Entity player;
	std::array<Entity,MAX_ROOM_NUM> rooms;
	std::array<Entity,MAX_ROOM_NUM * ENEM_NUM_PER_ROOM> roomEnemies;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
