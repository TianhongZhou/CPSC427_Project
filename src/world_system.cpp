// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

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
	: points(0), next_turtle_spawn(0.f), next_fish_spawn(0.f)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		// Mix_FreeChunk(salmon_dead_sound);
	if (player_attack_sound != nullptr)
		Mix_FreeChunk(player_attack_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
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
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto mouse_button_redirect = [](GLFWwindow *wnd, int button, int action, int mods)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_click(button, action, mods); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("Pinball Music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	player_attack_sound = Mix_LoadWAV(audio_path("Attack Sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || player_attack_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("Pinball Music.mp3").c_str(),
				audio_path("salmon_dead.wav").c_str(),
				audio_path("Attack Sound.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f)
		{
			if (!registry.players.has(motion_container.entities[i])) // don't remove the player
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	// Spawning new turtles
	// next_turtle_spawn -= elapsed_ms_since_last_update * current_speed;
	// if (registry.rooms.components.size() <= MAX_TURTLES && next_turtle_spawn < 0.f) {
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

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

	float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities)
	{
		// progress timer
		DeathTimer &timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms)
		{
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0)
		{
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
void WorldSystem::restart_game()
{
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

	// Create a new salmon
	rooms[0] = createRoom(renderer, { 600, 400 });
	player = createPlayer(renderer, { 350, 200 });
	registry.lights.emplace(player);
}

void WorldSystem::init_combat(int initCombat)
{

	//Entity player_ball = createBall(renderer, {400, 400});
	//createNewRectangleTiedToEntity(player_ball, 12.f, 12.f, registry.motions.get(player_ball).position);

	//wall
	Entity leftwall = createPolygonByVertex(renderer, {{220, 749}, {220, 1}, {240, 1}, {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
	createNewRectangleTiedToEntity(leftwall, 20.f, 698.f, registry.motions.get(leftwall).position, false, 1.0);

	Entity rightwall = createPolygonByVertex(renderer, { {820, 749}, {820, 1}, {840, 1}, {840, 749} }, GEOMETRY_BUFFER_ID::OCT);
	createNewRectangleTiedToEntity(rightwall, 20.f, 698.f, registry.motions.get(rightwall).position, false, 1.0);

	//Entity flipper = createPolygonByVertex(renderer, {{300, 600}, {300, 580}, {400, 580}, {400, 600}}, GEOMETRY_BUFFER_ID::RECT);

	//ball
	Entity squareball = createPolygonByVertex(renderer, { {500, 270}, {500, 220}, {550, 220}, {550, 270} }, GEOMETRY_BUFFER_ID::OCT);
	createNewRectangleTiedToEntity(squareball, 50.f, 50.f, registry.motions.get(squareball).position, true, 1.0);


	//slide
	Entity leftslide = createPolygonByVertex(renderer, { {220, 750}, {220, 730}, {400, 730}, {400, 750} }, GEOMETRY_BUFFER_ID::RECT);
	createNewRectangleTiedToEntity(leftslide, 180.f, 20.f, registry.motions.get(leftslide).position, false, 1.0);

	Entity rightslide = createPolygonByVertex(renderer, { {660, 750}, {660, 730}, {840, 730}, {840, 750} }, GEOMETRY_BUFFER_ID::RECT);
	createNewRectangleTiedToEntity(rightslide, 180.f, 20.f, registry.motions.get(rightslide).position, false, 1.0);


	//flipper
	Entity flipper = createPolygonByVertex(renderer, {{480, 600}, {480, 580}, {580, 580}, {580, 600}}, GEOMETRY_BUFFER_ID::RECT);
	createNewRectangleTiedToEntity(flipper, 100.f, 20.f, registry.motions.get(flipper).position, true, 0.0);


	//enemy
	Entity enemyobj = createPolygonByVertex(renderer, { {460, 170}, {460, 120}, {580, 120}, {580, 170} }, GEOMETRY_BUFFER_ID::OCT);
	createNewRectangleTiedToEntity(enemyobj, 120.f, 50.f, registry.motions.get(enemyobj).position, false, 1.0);
	PinBallEnemy &pinballEnemy = registry.pinballEnemies.emplace(enemyobj);
	pinballEnemy.maxBlood = 100.0f;
	pinballEnemy.currentBlood = 100.0f;

	playerFlipper pf;
	registry.playerFlippers.insert(flipper, pf);

	Entity pinballenemyBloodBg = createPinBallEnemyBlood(renderer, { 520, 50 });
	registry.colors.insert(pinballenemyBloodBg, { 0.2, 0.2, 0.2 });

	Entity pinballenemyBlood = createPinBallEnemyBlood(renderer, { 520, 50 });
	registry.colors.insert(pinballenemyBlood, { 1, 0, 0 });
	registry.bloods.emplace(pinballenemyBlood);
}

// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// For now, we are only interested in collisions that involve the salmon
		if (registry.players.has(entity))
		{
			// Player& player = registry.players.get(entity);

			// Checking Player - Room collisions
			if (registry.rooms.has(entity_other))
			{
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity))
				{
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);

					// !!! TODO A1: change the salmon orientation and color on death
				}
			}
			// Checking Player - mainWorldEnemies collisions
			else if (registry.mainWorldEnemies.has(entity_other))
			{
				if (!registry.deathTimers.has(entity))
				{
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					// Mix_PlayChannel(-1, player_attack_sound, 0);
					++points;

				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{

	if (action == GLFW_PRESS)
	{
		pressedKeys.insert(key);
	}

	if (action == GLFW_RELEASE)
	{
		pressedKeys.erase(key);
	}

	Motion &motion = registry.motions.get(player);
	bool inCombat = GameSceneState || registry.enterCombatTimer.has(player);;

	bool conflictUpAndDown = pressedKeys.count(GLFW_KEY_UP) && pressedKeys.count(GLFW_KEY_DOWN);
	bool conflictLeftAndRight = pressedKeys.count(GLFW_KEY_LEFT) && pressedKeys.count(GLFW_KEY_RIGHT);

	if (!conflictUpAndDown && (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
	{
		motion.velocity.y = (key == GLFW_KEY_UP) ? -200.f : 200.f;
	}
	else if (conflictUpAndDown)
	{
		motion.velocity.y = 0.f;
	}

	if (!conflictLeftAndRight && (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
	{
		motion.velocity.x = (key == GLFW_KEY_LEFT) ? -200.f : 200.f;
	}
	else if (conflictLeftAndRight)
	{
		motion.velocity.x = 0.f;
	}

	if (action == GLFW_RELEASE)
	{
		if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && !conflictUpAndDown)
		{
			if (pressedKeys.count(GLFW_KEY_UP) && GameSceneState == 0)
			{
				motion.velocity.y = -200.f;
			}
			else if (pressedKeys.count(GLFW_KEY_DOWN) && GameSceneState == 0)
			{
				motion.velocity.y = 200.f;
			}
			else
			{
				motion.velocity.y = 0.f;
			}
		}

		if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && !conflictLeftAndRight)
		{
			if (pressedKeys.count(GLFW_KEY_LEFT) && GameSceneState == 0)
			{
				motion.velocity.x = -200.f;
			}
			else if (pressedKeys.count(GLFW_KEY_RIGHT) && GameSceneState == 0)
			{
				motion.velocity.x = 200.f;
			}
			else
			{
				motion.velocity.x = 0.f;
			}
		}
	}

	if ((motion.velocity.x == 0.f) && (motion.velocity.y == 0.f))
	{
		if (registry.spriteSheets.has(player))
		{
			SpriteSheet &spriteSheet = registry.spriteSheets.get(player);
			RenderRequest &renderRequest = registry.renderRequests.get(player);
			renderRequest.used_texture = spriteSheet.origin;
			registry.spriteSheets.remove(player);
		}
	}
	else
	{
		if (!registry.spriteSheets.has(player))
		{
			SpriteSheet &spriteSheet = registry.spriteSheets.emplace_with_duplicates(player);
			spriteSheet.next_sprite = TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET;
			spriteSheet.frameIncrement = 0.06f;
			spriteSheet.frameAccumulator = 0.0f;
			spriteSheet.spriteSheetHeight = 1;
			spriteSheet.spriteSheetWidth = 6;
			spriteSheet.totalFrames = 6;
			spriteSheet.origin = TEXTURE_ASSET_ID::PLAYER;
			spriteSheet.loop = true;
			if (motion.velocity.x < 0.f)
			{
				spriteSheet.xFlip = true;
			}
			RenderRequest &renderRequest = registry.renderRequests.get(player);
			renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET;
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Exit Combat
	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_X)
	{
		exit_combat();
	}

	// Enter Combat
	if (GameSceneState == 0 && action == GLFW_RELEASE && key == GLFW_KEY_C)
	{
		GameSceneState = 1;
		InitCombat = 1;
	}

	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_P)
	{
		Entity &flipper = registry.playerFlippers.entities[0];

		physObj &flipperPhys = registry.physObjs.get(flipper);

		flipperPhys.Vertices[3].accel += vec2(0.f, -0.8f);
	}

	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
	{
		Entity &flipper = registry.playerFlippers.entities[0];

		physObj &flipperPhys = registry.physObjs.get(flipper);

		flipperPhys.Vertices[1].accel += vec2(0.1f, 0.f);
	}

	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
	{
		Entity &flipper = registry.playerFlippers.entities[0];

		physObj &flipperPhys = registry.physObjs.get(flipper);

		flipperPhys.Vertices[1].accel += vec2(-0.1f, 0.f);
	}

	// Debugging
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
	{
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
	{
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	if (registry.mousePosArray.size() == 0) {
		Entity e;
		mousePos mp;
		mp.pos = mouse_position;
		registry.mousePosArray.insert(e, mp);
	}

	registry.mousePosArray.components[0].pos = mouse_position;

	(vec2) mouse_position; // dummy to avoid compiler warning
}

void WorldSystem::on_mouse_click(int button, int action, int mods)
{

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT && GameSceneState == 0)
	{
		Mix_PlayChannel(-1, player_attack_sound, 0);

		bool temp = false;
		if (registry.spriteSheets.has(player))
		{
			SpriteSheet &spriteSheet = registry.spriteSheets.get(player);
			if (spriteSheet.next_sprite == TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET)
			{
				return;
			}
			temp = spriteSheet.xFlip;
		}
		SpriteSheet &spriteSheet = registry.spriteSheets.emplace_with_duplicates(player);
		spriteSheet.next_sprite = TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET;
		spriteSheet.frameIncrement = 0.08f;
		spriteSheet.frameAccumulator = 0.0f;
		spriteSheet.spriteSheetHeight = 1;
		spriteSheet.spriteSheetWidth = 6;
		spriteSheet.totalFrames = 6;
		spriteSheet.origin = TEXTURE_ASSET_ID::PLAYER;
		spriteSheet.xFlip = temp;
		RenderRequest &renderRequest = registry.renderRequests.get(player);
		renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET;

	}
}

void WorldSystem::exit_combat() {
	while (registry.combat.entities.size() > 0)
		registry.remove_all_components_of(registry.combat.entities.back());

    GameSceneState = 0;
}

// ================================================== WORLD ===============================================================================

// World functions

// Update our game world
bool WorldSystem::step_world(float elapsed_ms_since_last_update)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f)
		{
			if (!registry.players.has(motion_container.entities[i])) // don't remove the player
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	// handling entering combat state
	float min_timer_ms = 3000.f;
	for (Entity entity : registry.enterCombatTimer.entities)
	{
		EnterCombatTimer &timer = registry.enterCombatTimer.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms)
		{
			min_timer_ms = timer.timer_ms;
		}
		if (timer.timer_ms < 0)
		{
			if (registry.mainWorldEnemies.has(entity))
			{
				registry.remove_all_components_of(entity);
			}
			GameSceneState = 1;
			InitCombat = 1;
			printf("\n%d\n", registry.enterCombatTimer.get(entity).engagedEnemeis.size());
			registry.enterCombatTimer.remove(entity);
		}
	}

	// get room motion
	Motion &roomMotion = registry.motions.get(rooms[0]);

	// Boundary check for player and enemy if they are in room boundary
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		if (registry.players.has(motion_container.entities[i]) || registry.mainWorldEnemies.has(motion_container.entities[i]))
		{
			Motion &motion = motion_container.components[i];
			if (roomMotion.position.x - (roomMotion.scale.x / 2) + 25.f > motion.position.x)
			{
				motion.position.x = roomMotion.position.x - (roomMotion.scale.x / 2) + 25.f;
			}
			else if (roomMotion.position.x + (roomMotion.scale.x / 2) - 25.f < motion.position.x)
			{
				motion.position.x = roomMotion.position.x + (roomMotion.scale.x / 2) - 25.f;
			}
			if (roomMotion.position.y - (roomMotion.scale.y / 2) + 25.f > motion.position.y)
			{
				motion.position.y = roomMotion.position.y - (roomMotion.scale.y / 2) + 25.f;
			}
			else if (roomMotion.position.y + (roomMotion.scale.y / 2) - 50.f < motion.position.y)
			{
				motion.position.y = roomMotion.position.y + (roomMotion.scale.y / 2) - 50.f;
			}
		}
	}

	for (Entity entity : registry.highLightEnemies.entities)
	{
		// progress timer
		HighLightEnemy &highLightEnemies = registry.highLightEnemies.get(entity);
		highLightEnemies.timer_ms -= elapsed_ms_since_last_update;
		if (highLightEnemies.timer_ms < min_timer_ms)
		{
			min_timer_ms = highLightEnemies.timer_ms;
		}

		// restart the game once the death timer expired
		if (highLightEnemies.timer_ms < 0)
		{
			registry.motions.get(entity).velocity.x = 90.f;
			registry.highLightEnemies.remove(entity);
			return true;
		}
	}

	return true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions_world()
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity playerC = collisionsRegistry.entities[i];
		Entity enemy = collisionsRegistry.components[i].other_entity;
		if (registry.players.has(playerC))
		{
			// Checking Player - Enemy collisions
			if (registry.mainWorldEnemies.has(enemy))
			{
				if (!registry.enterCombatTimer.has(playerC))
				{
					registry.enterCombatTimer.emplace(playerC);
					registry.enterCombatTimer.get(playerC).engagedEnemeis.push_back(enemy);
				} else {
					std::vector<Entity> vec = registry.enterCombatTimer.get(playerC).engagedEnemeis;
					int find = 0;
					for (unsigned int j = 0; j < vec.size(); j++) {
    					if (vec[j]==enemy) {
							find = 1;
							break;
						}
  					}
					if (!find) {
        				registry.enterCombatTimer.get(playerC).engagedEnemeis.push_back(enemy);
    				}
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}