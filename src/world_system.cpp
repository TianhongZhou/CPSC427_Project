// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;

// Game state global variables                            
int GameSceneState = 0;
int InitCombat = 0;

// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
	, next_turtle_spawn(0.f)
	, next_fish_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int button, int action, int mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(button, action, mods); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("salmon_dead.wav").c_str(),
			audio_path("salmon_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size()-1; i>=0; --i) {
	    Motion& motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motion_container.entities[i])) // don't remove the player
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	// Spawning new turtles
	//next_turtle_spawn -= elapsed_ms_since_last_update * current_speed;
	//if (registry.rooms.components.size() <= MAX_TURTLES && next_turtle_spawn < 0.f) {
	//	// Reset timer
	//	next_turtle_spawn = (TURTLE_DELAY_MS / 2) + uniform_dist(rng) * (TURTLE_DELAY_MS / 2);
	//	// Create turtle
	//	Entity entity = createTurtle(renderer, {0,0});
	//	// Setting random initial position and constant velocity
	//	Motion& motion = registry.motions.get(entity);
	//	motion.position =
	//		vec2(window_width_px -200.f,
	//			 50.f + uniform_dist(rng) * (window_height_px - 100.f));
	//	motion.velocity = vec2(-100.f, 0.f);
	//}


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if(timer.timer_ms < min_timer_ms){
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.screen_darken_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present salmons is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;


	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	GameSceneState = 0; // reset to world scene (we can make a function for combat restart)

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	rooms[0] = createRoom(renderer, { 600, 400 });
	roomEnemies[0] = createRoomEnemy(renderer, { 600, 400 });
	player = createPlayer(renderer, { 350, 200 });
	registry.colors.insert(roomEnemies[0], { 1, 0, 0 });
	// player_salmon = createSalmon(renderer, { 10, 20 });
	// registry.colors.insert(player_salmon, {1, 0.8f, 0.8f});



	//Entity player_ball = createBall(renderer, { 400, 400 });
	//// createNewRectangleTiedToEntity(player_ball, 12.f, 12.f, registry.motions.get(player_ball).position);

	//Entity rectangle2 = createPolygonByVertex(renderer, { {220, 350}, { 220,220 }, { 300,220 }, { 300,350 } }, GEOMETRY_BUFFER_ID::OCT);

	//createNewRectangleTiedToEntity(rectangle2, 80.f, 130.f, registry.motions.get(rectangle2).position);

	//physObj test = registry.physObjs.components[0];


	//Entity rectangle = createPolygonByVertex(renderer, { {220, 350}, { 220,220 }, { 400,220 }, { 400,350 } }, GEOMETRY_BUFFER_ID::RECT);

	//createNewRectangleTiedToEntity(rectangle, 180.f, 130.f, registry.motions.get(rectangle).position -vec2(0.0,150.0));
	//

	//test = registry.physObjs.components[0];

	//physObj test2 = registry.physObjs.components[1];


	////Entity oct = createPolygonByVertex(renderer, {
	////{680.0f, 400.0f},
	////{660.56f, 460.56f},
	////{600.0f, 480.0f}, 
	////{539.44f, 460.56f}, 
	////{520.0f, 400.0f},
	////{539.44f, 339.44f},
	////{600.0f, 320.0f},
	////{660.56f, 339.44f}
	////	}, GEOMETRY_BUFFER_ID::OCT);
	//Entity pinballenemy = createPinBallEnemy(renderer, { 100, 700 });
	//registry.colors.insert(pinballenemy, { 1, 0, 0 });
	//Entity enemyWave = createEnemyWave(renderer, { 400, 600 });
	//registry.colors.insert(enemyWave, { 0, 0, 1 });
	//Entity room = createRoom(renderer, { 800, 400 });
	//Entity road = createRoad(renderer, { 800, 200 });
	//player = createPlayer(renderer, { 800, 600 });
	//Entity roomEnemy = createRoomEnemy(renderer, { 1000, 600 });
	//registry.colors.insert(roomEnemy, { 1, 0, 0 });
}


void WorldSystem::init_combat() {

	Entity player_ball = createBall(renderer, { 400, 400 });
	// createNewRectangleTiedToEntity(player_ball, 12.f, 12.f, registry.motions.get(player_ball).position);

	Entity rectangle2 = createPolygonByVertex(renderer, { {220, 350}, { 220,220 }, { 300,220 }, { 300,350 } }, GEOMETRY_BUFFER_ID::OCT);

	createNewRectangleTiedToEntity(rectangle2, 80.f, 130.f, registry.motions.get(rectangle2).position);

	physObj test = registry.physObjs.components[0];


	Entity rectangle = createPolygonByVertex(renderer, { {220, 350}, { 220,220 }, { 400,220 }, { 400,350 } }, GEOMETRY_BUFFER_ID::RECT);

	createNewRectangleTiedToEntity(rectangle, 180.f, 130.f, registry.motions.get(rectangle).position - vec2(0.0, 150.0));


	test = registry.physObjs.components[0];

	physObj test2 = registry.physObjs.components[1];


	//Entity oct = createPolygonByVertex(renderer, {
	//{680.0f, 400.0f},
	//{660.56f, 460.56f},
	//{600.0f, 480.0f}, 
	//{539.44f, 460.56f}, 
	//{520.0f, 400.0f},
	//{539.44f, 339.44f},
	//{600.0f, 320.0f},
	//{660.56f, 339.44f}
	//	}, GEOMETRY_BUFFER_ID::OCT);
	Entity pinballenemy = createPinBallEnemy(renderer, { 100, 700 });
	registry.colors.insert(pinballenemy, { 1, 0, 0 });
	Entity enemyWave = createEnemyWave(renderer, { 400, 600 });
	registry.colors.insert(enemyWave, { 0, 0, 1 });

}


// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// For now, we are only interested in collisions that involve the salmon
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Room collisions
			if (registry.rooms.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);

					// !!! TODO A1: change the salmon orientation and color on death
				}
			}
			// Checking Player - mainWorldEnemies collisions
			else if (registry.mainWorldEnemies.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, salmon_eat_sound, 0);
					++points;

					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the salmon entity by modifying the ECS registry
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	
	Motion& motion = registry.motions.get(player);

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_UP) {
		motion.velocity.y = -200.f;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_UP) {
		motion.velocity.y = 0.f;
	}

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_DOWN) {
		motion.velocity.y = 200.f;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_DOWN) {
		motion.velocity.y = 0.f;
	}

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_LEFT) {
		motion.velocity.x = -200.f;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT) {
		motion.velocity.x = 0.f;
	}

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_RIGHT) {
		motion.velocity.x = 200.f;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_RIGHT) {
		motion.velocity.x = 0.f;
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

    // Exit Combat
    if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_X) {
        exit_combat();
    }

	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_P) {
		registry.physObjs.components[0].Vertices[0].accel = vec2(0.0, -0.1);
		
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	(vec2)mouse_position; // dummy to avoid compiler warning
}

void WorldSystem::on_mouse_click(int button, int action, int mods) {

	if (GameSceneState == 1) {
		if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
			RenderRequest& renderRequest = registry.renderRequests.get(player);
			renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERATTACK;
		}

		if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
			RenderRequest& renderRequest = registry.renderRequests.get(player);
			renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYER;
		}
	}
}

void WorldSystem::exit_combat() {
    GameSceneState = 0;
    rooms[0] = createRoom(renderer, { 600, 400 });
    player = createPlayer(renderer, { 350, 200 });
}


// ================================================== WORLD ===============================================================================

// World functions

// Update our game world
bool WorldSystem::step_world(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i) {
		Motion& motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if (!registry.players.has(motion_container.entities[i])) // don't remove the player
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	//// Spawning new turtles
	//next_turtle_spawn -= elapsed_ms_since_last_update * current_speed;
	//if (registry.rooms.components.size() <= MAX_TURTLES && next_turtle_spawn < 0.f) {
	//	// Reset timer
	//	next_turtle_spawn = (TURTLE_DELAY_MS / 2) + uniform_dist(rng) * (TURTLE_DELAY_MS / 2);
	//	// Create turtle
	//	Entity entity = createTurtle(renderer, {0,0});
	//	// Setting random initial position and constant velocity
	//	Motion& motion = registry.motions.get(entity);
	//	motion.position =
	//		vec2(window_width_px + 100.f, // 2c: spawn outside of screen
	//			 50.f + uniform_dist(rng) * (window_height_px - 100.f));
	//	motion.velocity = vec2(-100.f, 0.f);
	//}

	// Spawning new fish
	// next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	// if (registry.mainWorldEnemies.components.size() <= MAX_FISH && next_fish_spawn < 0.f) {
	// 	// !!!  TODO A1: Create new fish with createFish({0,0}), as for the Turtles above

	// 	//2c
	// 	// Reset timer
	// 	next_fish_spawn = (FISH_DELAY_MS / 2) + uniform_dist(rng) * (FISH_DELAY_MS / 2);
	// 	// Create fish
	// 	Entity entity = createFish(renderer, { 0,0 });
	// 	// Setting random initial position and constant velocity
	// 	Motion& motion = registry.motions.get(entity);
	// 	motion.position =
	// 		vec2(window_width_px + 100.f, // 2c: spawn outside of screen
	// 			50.f + uniform_dist(rng) * (window_height_px - 100.f));
	// 	motion.velocity = vec2(-200.f, 0.f);
	// }
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO A2: HANDLE PEBBLE SPAWN HERE
// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// // Processing the salmon state
// 	assert(registry.screenStates.components.size() <= 1);
// 	ScreenState& screen = registry.screenStates.components[0];

// 	float min_timer_ms = 3000.f;
// 	for (Entity entity : registry.deathTimers.entities) {
// 		// progress timer
// 		DeathTimer& timer = registry.deathTimers.get(entity);
// 		timer.timer_ms -= elapsed_ms_since_last_update;
// 		if (timer.timer_ms < min_timer_ms) {
// 			min_timer_ms = timer.timer_ms;
// 		}

// 		// restart the game once the death timer expired
// 		if (timer.timer_ms < 0) {
// 			registry.deathTimers.remove(entity);
// 			screen.screen_darken_factor = 0;
// 			restart_game();
// 			return true;
// 		}
// 	}
// 	// reduce window brightness if any of the present salmons is dying
// 	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	//Boundary check for player and enemy if they are in room boundary
	Motion& playerMotion = registry.motions.get(player);
	Motion& roomMotion = registry.motions.get(rooms[0]);
	if (roomMotion.position.x - (roomMotion.scale.x/2) + 75.f > playerMotion.position.x) {
		playerMotion.position.x = roomMotion.position.x - (roomMotion.scale.x/2) + 75.f;
	} else if (roomMotion.position.x + (roomMotion.scale.x/2) - 25.f < playerMotion.position.x) {
		playerMotion.position.x = roomMotion.position.x + (roomMotion.scale.x/2) - 25.f;
	}
	if (roomMotion.position.y - (roomMotion.scale.y/2) + 75.f > playerMotion.position.y) {
		playerMotion.position.y = roomMotion.position.y - (roomMotion.scale.y/2) + 75.f;
	} else if (roomMotion.position.y + (roomMotion.scale.y/2) - 100.f < playerMotion.position.y) {
		playerMotion.position.y = roomMotion.position.y + (roomMotion.scale.y/2) - 100.f;
	}


	return true;
}



// Compute collisions between entities
void WorldSystem::handle_collisions_world() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;
		if (registry.players.has(entity)) {
			// Checking Player - Enemy collisions
			if (registry.mainWorldEnemies.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// registry.remove_all_components_of(entity); //remove salmon from world
					registry.remove_all_components_of(entity_other);
					// registry.remove_all_components_of(entity);
					registry.remove_all_components_of(rooms[0]);
					main_world_out();
					GameSceneState = 1;
					InitCombat = 1;
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

//set main world stuff out of sight
void WorldSystem::main_world_out() {
	//set enemies, player, room, road invisible

}