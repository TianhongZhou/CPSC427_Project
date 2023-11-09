// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"
#include "pinball_system.hpp"

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
    redirect_inputs_world();

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

void WorldSystem::redirect_inputs_world() {
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

//// MOVED TO PinballSystem::step
// Update our game world
//bool WorldSystem::step(float elapsed_ms_since_last_update)
//{
//	// Entity blood = registry.healthBar.entities[0];
//	// Motion& motion = registry.motions.get(blood);
//	// if (motion.scale.x <= 0.5f)
//	// {
//	// 	GameSceneState = 0;
//	// }
//	if (registry.pinballEnemies.entities.size()==0) {
//		// exit_combat();
//		// GameSceneState = 0;
//	} else {
//		for (Entity entity: registry.pinballEnemies.entities) {
//		PinBallEnemy& enemy = registry.pinballEnemies.get(entity);
//		if (enemy.currentHealth<=0) {
//			for (int j=0; j<enemy.healthBar.size(); j++) {
//				registry.remove_all_components_of(enemy.healthBar[j]);
//			}
//			registry.remove_all_components_of(entity);
//		}
//	}
//	}
//
//	return true;
//}

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

	rooms[0] = createRoom(renderer, { 600, 400 });
	player = createPlayer(renderer, { 350, 200 });
	registry.lights.emplace(player);


	//int w, h;
	//glfwGetWindowSize(window, &w, &h);
	//// Add a ball
	//Entity entity = createPebble({ 0,0 }, { 0,0 }); //intialized below
	//Motion& motion = registry.motions.get(entity);
	//motion.position = {400, 400};
	//motion.scale = {30, 30};
	//motion.angle = 0;
	//motion.velocity = { 200, 200 };


	//registry.colors.insert(entity, { 1, 1, 1 });
}

//// MOVED TO PinballSystem::init_combat
void WorldSystem::init_combat(int initCombat, PinballSystem pinballSystem)
{
    pinballSystem.init(window, renderer, this);


//	// int w, h;
//	// glfwGetWindowSize(window, &w, &h);
//	vec2 boundary = { 260+50, 800-50 };
//	std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
//    std::uniform_real_distribution<float> distribution2(0.f, 1.f);
//
//	for (int i=0; i<initCombat; i++) {
//		Entity pinballenemy = createPinBallEnemy(renderer, {distribution1(gen),180*(i+1)}, boundary);
//		registry.colors.insert(pinballenemy, { distribution2(gen), distribution2(gen), distribution2(gen) });
//	}
//
//	Entity player_ball = createBall(renderer, {400, 400});
//	createNewRectangleTiedToEntity(player_ball, 30.f, 30.f, registry.motions.get(player_ball).position, true, 0.7);
//
//	//wall
//	Entity leftwall = createPolygonByVertex(renderer, {{220, 749}, {220, 1}, {240, 1}, {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
//	createNewRectangleTiedToEntity(leftwall, 20.f, 748.f, registry.motions.get(leftwall).position, false, 1.0);
//
//	Entity rightwall = createPolygonByVertex(renderer, { {820, 749}, {820, 1}, {840, 1}, {840, 749} }, GEOMETRY_BUFFER_ID::OCT);
//	createNewRectangleTiedToEntity(rightwall, 20.f, 748.f, registry.motions.get(rightwall).position, false, 1.0);
//
//	//Entity flipper = createPolygonByVertex(renderer, {{300, 600}, {300, 580}, {400, 580}, {400, 600}}, GEOMETRY_BUFFER_ID::RECT);
//
//	//ball
//	// Entity squareball = createPolygonByVertex(renderer, { {500, 570}, {500, 520}, {550, 520}, {550, 570} }, GEOMETRY_BUFFER_ID::OCT);
//	// createNewRectangleTiedToEntity(squareball, 50.f, 50.f, registry.motions.get(squareball).position, true, 1.0);
//
//
//	//slide
//	Entity leftslide = createPolygonByVertex(renderer, { {220, 750}, {220, 730}, {400, 750}, {400, 730} }, GEOMETRY_BUFFER_ID::RECT);
//	createNewRectangleTiedToEntity(leftslide, 180.f, 20.f, registry.motions.get(leftslide).position, false, 1.0);
//
//	Entity rightslide = createPolygonByVertex(renderer, { {660, 750}, {660, 730}, {840, 750}, {840, 730} }, GEOMETRY_BUFFER_ID::RECT);
//	createNewRectangleTiedToEntity(rightslide, 180.f, 20.f, registry.motions.get(rightslide).position, false, 1.0);
//
//
//	//flipper
//	Entity flipper = createPolygonByVertex(renderer, {{480, 600}, {480, 580}, {580, 580}, {580, 600}}, GEOMETRY_BUFFER_ID::RECT);
//	createNewRectangleTiedToEntity(flipper, 100.f, 20.f, registry.motions.get(flipper).position, true, 0.0);
//
//
//	//enemy
//	// Entity enemyobj = createPolygonByVertex(renderer, { {360, 380}, {360, 320}, {520, 320}, {520, 380} }, GEOMETRY_BUFFER_ID::OCT);
//	// createNewRectangleTiedToEntity(enemyobj, 120.f, 50.f, registry.motions.get(enemyobj).position, false, 1.0);
//	// PinBallEnemy &pinballEnemy = registry.pinballEnemies.emplace(enemyobj);
//	// registry.colors.insert(enemyobj, { 0.6, 0, 0 });
//
//	playerFlipper pf;
//	registry.playerFlippers.insert(flipper, pf);
}

// Compute collisions between entities
//void WorldSystem::handle_collisions()
//{
//	// Loop over all collisions detected by the physics system
//	auto &collisionsRegistry = registry.collisions;
//	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
//	{
//		// The entity and its collider
//		Entity entity = collisionsRegistry.entities[i];
//		Entity entity_other = collisionsRegistry.components[i].other_entity;
//
//		// For now, we are only interested in collisions that involve the salmon
//		if (registry.players.has(entity))
//		{
//			// Player& player = registry.players.get(entity);
//
//			// Checking Player - Room collisions
//			if (registry.rooms.has(entity_other))
//			{
//				// initiate death unless already dying
//				if (!registry.deathTimers.has(entity))
//				{
//					// Scream, reset timer, and make the salmon sink
//					registry.deathTimers.emplace(entity);
//					Mix_PlayChannel(-1, salmon_dead_sound, 0);
//
//				}
//			}
//			// Checking Player - mainWorldEnemies collisions
//			else if (registry.mainWorldEnemies.has(entity_other))
//			{
//				if (!registry.deathTimers.has(entity))
//				{
//					// chew, count points, and set the LightUp timer
//					registry.remove_all_components_of(entity_other);
//					// Mix_PlayChannel(-1, player_attack_sound, 0);
//					++points;
//
//				}
//			}
//		}
//	}
//
//	// Remove all collisions from this simulation step
//	registry.collisions.clear();
//}

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

	Motion& motion = registry.motions.get(player);
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
			SpriteSheet& spriteSheet = registry.spriteSheets.get(player);
			RenderRequest& renderRequest = registry.renderRequests.get(player);
			renderRequest.used_texture = spriteSheet.origin;
			registry.spriteSheets.remove(player);
		}
	}
	else
	{
		if (!registry.spriteSheets.has(player))
		{
			SpriteSheet& spriteSheet = registry.spriteSheets.emplace_with_duplicates(player);
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
			RenderRequest& renderRequest = registry.renderRequests.get(player);
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

//	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_P)
//	{
//		Entity& flipper = registry.playerFlippers.entities[0];
//
//		physObj& flipperPhys = registry.physObjs.get(flipper);
//
//		flipperPhys.Vertices[3].accel += vec2(0.f, -0.8f);
//	}
//
//	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
//	{
//		Entity& flipper = registry.playerFlippers.entities[0];
//
//		physObj& flipperPhys = registry.physObjs.get(flipper);
//
//		flipperPhys.Vertices[1].accel += vec2(0.1f, 0.f);
//	}
//
//	if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
//	{
//		Entity& flipper = registry.playerFlippers.entities[0];
//
//		physObj& flipperPhys = registry.physObjs.get(flipper);
//
//		flipperPhys.Vertices[1].accel += vec2(-0.1f, 0.f);
//	}

	// Debugging
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}
//
//	// Control the current speed with `<` `>`
//	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
//	{
//		current_speed -= 0.1f;
//		printf("Current speed = %f\n", current_speed);
//	}
//	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
//	{
//		current_speed += 0.1f;
//		printf("Current speed = %f\n", current_speed);
//	}
//	current_speed = fmax(0.f, current_speed);

	// player attack
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && GameSceneState == 0) {
		on_mouse_click(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
	}
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


		// Create player bullet
		Entity entity = createPlayerBullet({ 0,0 }, { 0,0 }); //intialized below

		Motion& motion = registry.motions.get(entity);
		motion.position = registry.motions.get(player).position;

		float radius = 20; //* (uniform_dist(rng) + 0.3f);
		motion.scale = { radius, radius };

		vec2 player_v = registry.motions.get(player).velocity;
		float angle = atan(player_v.y / player_v.x);

		// Set bullet angle and save last angle
		if (player_v.x == 0.f && player_v.y == 0.f) {
			angle = last_angle;
		}
		else {
			// CHECK: This is to fix flipping of the axis
			if (player_v.x < 0) {
				angle += atan(1) * 4;
			}
			last_angle = angle;
		}
		motion.angle = angle;


		//motion.velocity = vec2(200.f + uniform_dist(rng)*200, 100.f - uniform_dist(rng)*200);
		//float angle = registry.motions.get(player).angle;
		motion.velocity = vec2(500.f, 0.f);
		//motion.velocity.x = velocity.x * cos(angle) + velocity.y * sin(angle);
		//motion.velocity.y = velocity.x * sin(angle) + velocity.y * cos(angle);

		registry.colors.insert(entity, { 1, 1, 1 });
	}
}

void WorldSystem::exit_combat() {
	while (registry.combat.entities.size() > 0)
		registry.remove_all_components_of(registry.combat.entities.back());

    redirect_inputs_world();
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
	// save player orientation for attack
	Motion& motion = registry.motions.get(player);
	if (motion.velocity.y != 0.f || motion.velocity.x != 0.f) {
		last_angle = atan(motion.velocity.y / motion.velocity.x);
		if (motion.velocity.x < 0) {
			last_angle += atan(1) * 4;
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
			for (Entity ene: registry.enterCombatTimer.get(entity).engagedEnemeis) {
            	registry.remove_all_components_of(ene);
            }
         	for (Enemy& remain: registry.mainWorldEnemies.components) {
          		remain.seePlayer = false;
         	}
			GameSceneState = 1;
			InitCombat = registry.enterCombatTimer.get(entity).engagedEnemeis.size();
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

	//Generate enemy
	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(-450.0f, 450.0f);
    std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);

	generate_enemy_timer += elapsed_ms_since_last_update / 1000.0f; // Convert elapsed time to seconds

    if (generate_enemy_timer >= 3.f) {
		vec2 pos = registry.motions.get(registry.rooms.entities[0]).position;
		if (registry.mainWorldEnemies.entities.size()< 8) {
			Entity ene = createRoomEnemy(renderer, { pos[0]+distribution1(gen), pos[1]+distribution1(gen) }, pos, 700.f, false);
			registry.colors.insert(ene, { distribution2(gen), distribution2(gen), distribution2(gen) });
		}
        // Reset the timer
        generate_enemy_timer = 0.0f;
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
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;
		if (registry.players.has(entity))
		{
			// Checking Player - Enemy collisions
			if (registry.mainWorldEnemies.has(entity_other) && registry.positionKeyFrames.has(entity_other) && registry.mainWorldEnemies.size() == 1)
			{
				if (!registry.enterCombatTimer.has(entity))
				{
					registry.enterCombatTimer.emplace(entity);
					registry.enterCombatTimer.get(entity).engagedEnemeis.push_back(entity_other);
				} else {
					std::vector<Entity> vec = registry.enterCombatTimer.get(entity).engagedEnemeis;
					int find = 0;
					for (unsigned int j = 0; j < vec.size(); j++) {
    					if (vec[j]== entity_other) {
							find = 1;
							break;
						}
  					}
					if (!find) {
        				registry.enterCombatTimer.get(entity).engagedEnemeis.push_back(entity_other);
    				}
				}
			}
		}
		// enemy bullet vs player bullet collision
		if (registry.enemyBullets.has(entity) && registry.playerBullets.has(entity_other) ||
			registry.enemyBullets.has(entity_other) && registry.playerBullets.has(entity)) {
			// remove enemy upon collision
			registry.remove_all_components_of(entity);
			registry.remove_all_components_of(entity_other);
		}

		// player bullet vs enemy collision
		if (registry.mainWorldEnemies.has(entity) && registry.playerBullets.has(entity_other) ||
			registry.mainWorldEnemies.has(entity_other) && registry.playerBullets.has(entity)) {
			// remove enemy upon collision
			if (!registry.positionKeyFrames.has(entity) && !registry.positionKeyFrames.has(entity_other)) {
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(entity_other);
			}
		}

		// enemyy bullet vs player collision
		if (registry.enemyBullets.has(entity) && registry.players.has(entity_other) ||
			registry.enemyBullets.has(entity_other) && registry.players.has(entity)) {
			// handle damage interaction (nothing for now)
			if (registry.enemyBullets.has(entity)) {
				registry.remove_all_components_of(entity);
			}
			else {
				registry.remove_all_components_of(entity_other);
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}