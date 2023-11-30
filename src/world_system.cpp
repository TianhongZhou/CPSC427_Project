// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"
#include "pinball_system.hpp"

// For saving/loading game state
#include <../ext/nlohmann/json.hpp>
#include <fstream>
#include <iostream>

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;

// Game state global variables
int GameSceneState = 0;
int InitCombat = 0;
int MonitorWidth;
int MonitorHeight;
float MonitorScreenRatio;
int offsetX;
int offsetY;
int scaledWidth;
int scaledHeight;

// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
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

	// Obtain monitor full screen size

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    // using secondary monitor for testing purposes
//    int count;
//    GLFWmonitor** monitors = glfwGetMonitors(&count);
//    printf("count: %d\n", count);
//    GLFWmonitor* primaryMonitor = monitors[0];

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	MonitorWidth = static_cast<int>(mode->width );
	MonitorHeight = static_cast<int>(mode->height);
//    printf("monitor width: %d\n", MonitorWidth);
//    printf("monitor height: %d\n", MonitorHeight);

    float windowAspectRatio = static_cast<float>(window_width_px) / window_height_px;
	float monitorAspectRatio = static_cast<float>(MonitorWidth) / MonitorHeight;
	offsetX = 0;
	offsetY = 0; // Offsets for letterboxing
	
	if (windowAspectRatio > monitorAspectRatio) {
		// Window is wider relative to its height than the monitor is
		// Scale based on width
		scaledWidth = MonitorWidth;
		scaledHeight = static_cast<int>(MonitorWidth / windowAspectRatio);
		offsetY = (MonitorHeight - scaledHeight) / 2;
	}
	else {
		// Window is taller relative to its width than the monitor is
		// Scale based on height
		scaledWidth = static_cast<int>((float)MonitorHeight * windowAspectRatio * 1.09f);
		scaledHeight = MonitorHeight;
		offsetX = (MonitorWidth - scaledWidth) / 2 ;
	}

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(MonitorWidth, MonitorHeight, "Pinball Luminary", primaryMonitor, nullptr);
	//window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
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


	enemy_death_sound = Mix_LoadWAV(audio_path("enemyDeathSound.wav").c_str());
	dash_sound = Mix_LoadWAV(audio_path("dashSound.wav").c_str());
	enemy_hit_sound = Mix_LoadWAV(audio_path("enemyHitSound.wav").c_str());
	flipper_sound = Mix_LoadWAV(audio_path("flipperSound.wav").c_str());
	player_hit_sound = Mix_LoadWAV(audio_path("playerHitSound.wav").c_str());

	Entity sounds = Entity();
	soundForPhys s;
	s.enemy_death_sound = enemy_death_sound;
	s.enemy_hit_sound = enemy_hit_sound;
	s.player_hit_sound = player_hit_sound;

	registry.sfx.emplace(sounds, s);
	

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

	// Load saved game state
	load_game(PROJECT_SOURCE_DIR + std::string("gameState.json"));
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	GameSceneState = 0; // reset to world scene (we can make a function for combat restart)

    // FIXME: awkward place to put this logic
    // reset combat level
    registry.combatLevel.components[0].counter = 1;

	// Reset the game speed
	current_speed = 1.f;

	// Reset room number
	curr_room = 0;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	int w, h;
	glfwGetWindowSize(window, &w, &h);

	// Create room, player, lighting
	rooms[0] = createRoom(renderer, { 600, 400 }, window, 0);
	playerHealth = 100.f;
	player = createPlayer(renderer, { (window_width_px) / 2, 4 * (window_height_px) / 5 }, playerHealth); // spawn at the bottom of room for now  
	registry.lights.emplace(player);
}


// Save game
using json = nlohmann::json;
void WorldSystem::save_game(const std::string& filename)
{
	json j;

	// Save room level
	j["level"] = curr_room;

	// Check if player is in combat
	if (registry.mainWorldEnemies.size() > 0 || registry.snipers.size() > 0 || registry.zombies.size() > 0 ) {
		j["inCombat"] = 1;
	}
	else { // Only save game state if not in combat
		j["inCombat"] = 0;

		// Save power up stats
		PinBall& pinball = registry.pinBalls.get(player);
		j["pinball"]["size"] = pinball.pinBallSize;
		j["pinball"]["damage"] = pinball.pinBallDamage;

		// Save position
		Motion& motion = registry.motions.get(player);
		j["player"]["position"]["x"] = motion.position.x;
		j["player"]["position"]["y"] = motion.position.y;

		// Save buff drops
		auto& dropBuffsRegistry = registry.dropBuffs;
		for (uint i = 0; i < dropBuffsRegistry.components.size(); i++)
		{
			Entity buff_entity = dropBuffsRegistry.entities[i];
			Motion& buff_motion = registry.motions.get(buff_entity);
			j["dropBuffs"][i]["position"]["x"] = buff_motion.position.x;
			j["dropBuffs"][i]["position"]["y"] = buff_motion.position.y;
			j["dropBuffs"][i]["id"] = dropBuffsRegistry.components[i].id;
		}
		j["buffSize"] = dropBuffsRegistry.components.size();
		std::cout << "Saved buffs." << std::endl;
	}

	std::ofstream file(filename);
	if (file.is_open()) {
		file << j.dump(4); // '4' is for pretty printing
		file.close();
		std::cout << "Position saved successfully." << std::endl;
	}
	else {
		std::cerr << "Unable to open file for writing." << std::endl;
	}
}

void WorldSystem::load_game(const std::string& filename)
{
	std::ifstream file(filename);
	if (file.is_open()) {
		json j;
		file >> j;

		if (j.contains("level")) {
			// Restore room level
			curr_room = j["level"].get<int>();

			// URGENT TODO: temporary fix for spikes created
			if (curr_room != 0) {
				while (registry.spikes.entities.size() > 0)
					registry.remove_all_components_of(registry.spikes.entities.back());
			}
		}

		if (j.contains("inCombat")) {

			// Restore power up stats
			if (j.contains("pinball")) {
				PinBall& pinball = registry.pinBalls.get(player);
				pinball.pinBallSize = j["pinball"]["size"].get<float>();
				pinball.pinBallDamage = j["pinball"]["damage"].get<float>();
			}

			auto& dropBuffsRegistry = registry.dropBuffs;
			// Remove current first, TODO: Maybe change this
			while (dropBuffsRegistry.entities.size() > 0)
				registry.remove_all_components_of(dropBuffsRegistry.entities.back());


			if (j.contains("player") && j["inCombat"].get<int>() == 0) {
				// Restore position
				Motion& motion = registry.motions.get(player);
				motion.position.x = j["player"]["position"]["x"].get<float>();
				motion.position.y = j["player"]["position"]["y"].get<float>();

				// Restore buff drops
				//auto& dropBuffsRegistry = registry.dropBuffs;

				//// Remove current first, TODO: Maybe change this
				//while (dropBuffsRegistry.entities.size() > 0)
				//	registry.remove_all_components_of(dropBuffsRegistry.entities.back());

				int buff_size = j["buffSize"].get<int>();
				for (uint i = 0; i < buff_size; i++)
				{
					int buff_id = j["dropBuffs"][i]["id"].get<int>();
					float buff_px = j["dropBuffs"][i]["position"]["x"].get<float>();
					float buff_py = j["dropBuffs"][i]["position"]["y"].get<float>();

					TEXTURE_ASSET_ID id = TEXTURE_ASSET_ID::DROPBALLSIZE;
					if (buff_id == 1) {
						id = TEXTURE_ASSET_ID::DROPBALLDAMAGE;
					}
					Entity drop = createDropBuff(renderer, { buff_px, buff_py }, id);
					DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
				}
			}
			else {
				rooms[0] = createRoom(renderer, { 600, 400 }, window, curr_room);
			}
		}

		file.close();
		std::cout << "Position loaded successfully." << std::endl;
	}
	else {
		std::cerr << "Unable to open file for reading." << std::endl;
	}
}




//// MOVED TO PinballSystem::restart
void WorldSystem::init_combat(PinballSystem pinballSystem)
{
    pinballSystem.init(window, renderer, this);

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

	Motion& motion = registry.motions.get(player);
//	bool inCombat = GameSceneState || registry.enterCombatTimer.has(player);;

	bool conflictUpAndDown = pressedKeys.count(GLFW_KEY_W) && pressedKeys.count(GLFW_KEY_S);
	bool conflictLeftAndRight = pressedKeys.count(GLFW_KEY_A) && pressedKeys.count(GLFW_KEY_D);

	if (!conflictUpAndDown && (key == GLFW_KEY_W || key == GLFW_KEY_S) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
	{
		motion.velocity.y = (key == GLFW_KEY_W) ? -200.f : 200.f;
	}
	else if (conflictUpAndDown)
	{
		motion.velocity.y = 0.f;
	}

	if (!conflictLeftAndRight && (key == GLFW_KEY_A || key == GLFW_KEY_D) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
	{
		motion.velocity.x = (key == GLFW_KEY_A) ? -200.f : 200.f;
	}
	else if (conflictLeftAndRight)
	{
		motion.velocity.x = 0.f;
	}

	if (action == GLFW_RELEASE)
	{
		if ((key == GLFW_KEY_W || key == GLFW_KEY_S) && !conflictUpAndDown)
		{
			if (pressedKeys.count(GLFW_KEY_W) && GameSceneState == 0)
			{
				motion.velocity.y = -200.f;
			}
			else if (pressedKeys.count(GLFW_KEY_S) && GameSceneState == 0)
			{
				motion.velocity.y = 200.f;
			}
			else
			{
				motion.velocity.y = 0.f;
			}
		}

		if ((key == GLFW_KEY_A || key == GLFW_KEY_D) && !conflictLeftAndRight)
		{
			if (pressedKeys.count(GLFW_KEY_A) && GameSceneState == 0)
			{
				motion.velocity.x = -200.f;
			}
			else if (pressedKeys.count(GLFW_KEY_D) && GameSceneState == 0)
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


	// Enter Combat
	if (GameSceneState == 0 && action == GLFW_RELEASE && key == GLFW_KEY_C)
	{
		GameSceneState = 1;
		InitCombat = 1;
	}


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

	// save game
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && GameSceneState == 0) {
		save_game(PROJECT_SOURCE_DIR + std::string("gameState.json")); // NOTE: assumes backslash at the end of PROJECT_SOURCE_DIR
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
		vec2 mouse_position = registry.mousePosArray.components[0].pos;
		float dy = mouse_position.y * (float)window_height_px / (float)MonitorHeight - motion.position.y;
		float dx = mouse_position.x * (float)window_width_px / (float)MonitorWidth - motion.position.x;
		motion.angle = atan2(dy, dx);


		//printf("This is mouse_position: %f, %f\n", mouse_position.x, mouse_position.y);
		//printf("This is player_position: %f, %f\n", motion.position.x, motion.position.y);

		//motion.velocity = vec2(200.f + uniform_dist(rng)*200, 100.f - uniform_dist(rng)*200);
		//float angle = registry.motions.get(player).angle;
		motion.velocity = vec2(500.f, 0.f);
		//motion.velocity.x = velocity.x * cos(angle) + velocity.y * sin(angle);
		//motion.velocity.y = velocity.x * sin(angle) + velocity.y * cos(angle);

		registry.colors.insert(entity, { 1, 1, 1 });
	}
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
	float step_seconds = elapsed_ms_since_last_update / 1000.f;

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motion_container = registry.motions;
	Motion& playerMotion = motion_container.get(registry.players.entities[0]);
	Player& player = registry.players.components[0];
	if (spike_damage_timer>0.f) spike_damage_timer-=step_seconds;
	playerHealth=player.currentHealth;
	if (player.currentHealth<=0) restart_game();
	// Health bar follows player
	for (int j=0; j<player.healthBar.size(); j++) {
		if (motion_container.has(player.healthBar[j])) {
			Motion& healthbarMotion = motion_container.get(player.healthBar[j]);
			healthbarMotion.position.x = playerMotion.position.x;
			healthbarMotion.position.y = playerMotion.position.y-50.f;
		}
	}
	
	// Update healthbar
	if (motion_container.has(player.healthBar[1]) && motion_container.has(player.healthBar[2])) {
		Motion& barMotion = motion_container.get(player.healthBar[0]);
		Motion& healthMotion = motion_container.get(player.healthBar[1]);
		Motion& amortizedMotion = motion_container.get(player.healthBar[2]);
		healthMotion.scale.x =  barMotion.scale.x * player.currentHealth/player.maxHealth;
		healthMotion.position.x = barMotion.position.x - (barMotion.scale.x - healthMotion.scale.x) / 2;
		if (amortizedMotion.scale.x>healthMotion.scale.x) {
			amortizedMotion.scale.x -= 0.5f;
		}
		amortizedMotion.position.x = barMotion.position.x - (barMotion.scale.x - amortizedMotion.scale.x) / 2;
	}
	
	save_player_last_direction();

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

	check_room_boundary();

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

	//spawn_room_enemies(elapsed_ms_since_last_update);

	//// TODO: move this to the top later
	//int w, h;
	//glfwGetWindowSize(window, &w, &h);

	// Enter next room
	//Motion& player_motion = registry.motions.get(player);
	//if (player_motion.position.y < 60.f && player_motion.position.x > w/2 - 40 && player_motion.position.x < w / 2 + 40)
	//	//&& registry.mainWorldEnemies.size()) 
	//{
	//	enter_next_room();
	//	player_motion.position = { w/2, h * 4/ 5};
	//}

	return true;
}

// Enter next room
void WorldSystem::enter_next_room()
{	
	// TODO: move this to the top later
	int w, h;
	glfwGetWindowSize(window, &w, &h);

	PinBall temp = registry.pinBalls.get(player);

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	curr_room += 1; 
	rooms[0] = createRoom(renderer, {600, 400}, window, curr_room);
	player = createPlayer(renderer, { (window_width_px) / 2, 4 * (window_height_px) / 5 }, playerHealth);

	PinBall& pinBall = registry.pinBalls.get(player);
	pinBall.pinBallSize = temp.pinBallSize;
	pinBall.pinBallDamage = temp.pinBallDamage;
	registry.lights.emplace(player);

}

// Generate enemies in room
// TODO: when multiple rooms are implemented, make sure it checks the right room
void WorldSystem::spawn_room_enemies(float elapsed_ms_since_last_update)
{
	//Generate enemy
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(-450.0f, 450.0f);
	std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);

	generate_enemy_timer += elapsed_ms_since_last_update / 1000.0f; // Convert elapsed time to seconds

	if (generate_enemy_timer >= 3.f) {
		vec2 pos = registry.motions.get(registry.rooms.entities[0]).position;
		if (registry.mainWorldEnemies.entities.size() < 8) {
			Entity ene = createRoomEnemy(renderer, { pos[0] + distribution1(gen), pos[1] + distribution1(gen) }, pos, 700.f, false);
			registry.colors.insert(ene, { distribution2(gen), distribution2(gen), distribution2(gen) });
		}
		// Reset the timer
		generate_enemy_timer = 0.0f;
	}
}


// Room boundary check
// TODO: when multiple rooms are implemented, make sure it checks the right room

void WorldSystem::check_room_boundary()
{	
	auto& motion_container = registry.motions;
	// get room motion
	Motion& roomMotion = registry.motions.get(rooms[0]);

	// Boundary check for player and enemy if they are in room boundary
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		if (registry.players.has(motion_container.entities[i]) || registry.mainWorldEnemies.has(motion_container.entities[i]))
		{
			Motion& motion = motion_container.components[i];
			if (roomMotion.position.x - (roomMotion.scale.x / 2) + 65.f > motion.position.x)
			{
				motion.position.x = roomMotion.position.x - (roomMotion.scale.x / 2) + 65.f;
			}
			else if (roomMotion.position.x + (roomMotion.scale.x / 2) - 45.f < motion.position.x)
			{
				motion.position.x = roomMotion.position.x + (roomMotion.scale.x / 2) - 45.f;
			}
			if (roomMotion.position.y - (roomMotion.scale.y / 2) + 70.f > motion.position.y)
			{
				motion.position.y = roomMotion.position.y - (roomMotion.scale.y / 2) + 70.f;
			}
			else if (roomMotion.position.y + (roomMotion.scale.y / 2) - 95.f < motion.position.y)
			{
				motion.position.y = roomMotion.position.y + (roomMotion.scale.y / 2) - 95.f;
			}
		}
	}
}

// Save player's last facing direction for projectile shooting
void WorldSystem::save_player_last_direction() 
{
	// save player orientation for attack
	Motion& motion = registry.motions.get(player);
	if (motion.velocity.y != 0.f || motion.velocity.x != 0.f) {
		last_angle = atan(motion.velocity.y / motion.velocity.x);
		if (motion.velocity.x < 0) {
			last_angle += atan(1) * 4;
		}
	}
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
		//if (registry.players.has(entity))
		//{
		//	// Checking Player - Enemy collisions
		//	if (registry.mainWorldEnemies.has(entity_other) && registry.positionKeyFrames.has(entity_other) && registry.mainWorldEnemies.size() == 1)
		//	{
		//		if (!registry.enterCombatTimer.has(entity))
		//		{
		//			registry.enterCombatTimer.emplace(entity);
		//			registry.enterCombatTimer.get(entity).engagedEnemeis.push_back(entity_other);
		//		} else {
		//			std::vector<Entity> vec = registry.enterCombatTimer.get(entity).engagedEnemeis;
		//			int find = 0;
		//			for (unsigned int j = 0; j < vec.size(); j++) {
  //  					if (vec[j]== entity_other) {
		//					find = 1;
		//					break;
		//				}
  //					}
		//			if (!find) {
  //      				registry.enterCombatTimer.get(entity).engagedEnemeis.push_back(entity_other);
  //  				}
		//		}
		//	}
		//}
		// enemy bullet vs player bullet collision
		if (registry.enemyBullets.has(entity) && registry.playerBullets.has(entity_other) ||
			registry.enemyBullets.has(entity_other) && registry.playerBullets.has(entity)) {
			// remove enemy upon collision
			registry.remove_all_components_of(entity);
			registry.remove_all_components_of(entity_other);
		}

		// player bullet vs enemy collision
		if (registry.mainWorldEnemies.has(entity) && registry.playerBullets.has(entity_other) ||
			registry.snipers.has(entity) && registry.playerBullets.has(entity_other) || 
			registry.zombies.has(entity) && registry.playerBullets.has(entity_other) ) {

			// remove enemy upon collision
			if (!registry.bosses.has(entity)) {
				GenerateDropBuff(entity);
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(entity_other);
			}
			else if (registry.mainWorldEnemies.size() == 1) {
				registry.motions.get(player).velocity = vec2(0.f, 0.f);
				GameSceneState = 1;
				InitCombat = 1;
				GenerateDropBuff(entity);
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(entity_other);
			}
		}

		// enemyy bullet vs player collision
		if (registry.enemyBullets.has(entity) && registry.players.has(entity_other) ||
			registry.zombies.has(entity) && registry.players.has(entity_other)) {
			// handle damage interaction (nothing for now)
			registry.players.components[0].currentHealth -= 10.f;
			if (registry.enemyBullets.has(entity)) {
				registry.remove_all_components_of(entity);
			}
			else if (registry.zombies.has(entity)) {
				registry.remove_all_components_of(entity);
			}
			else {
				registry.remove_all_components_of(entity_other);
			}
		}

		// spike collision
		if (registry.spikes.has(entity) && registry.players.has(entity_other)) {
			// handle damage interaction (nothing for now)
			if (spike_damage_timer<=0.f) {
				registry.players.components[0].currentHealth -= 10.f;
				spike_damage_timer=1.f;
			}
		}

		// player vs door collision
		if (registry.doors.has(entity) && registry.players.has(entity_other)) {
			// handle damage interaction (nothing for now)
			enter_next_room();
		}

		// drop buff vs player collision
		if (registry.dropBuffs.has(entity) && registry.players.has(entity_other) ||
			registry.dropBuffs.has(entity_other) && registry.players.has(entity)) {
			if (registry.dropBuffs.has(entity)) {
				DropBuffAdd(registry.dropBuffs.get(entity));
				registry.remove_all_components_of(entity);
			}
			else if (registry.dropBuffs.has(entity_other)) {
				DropBuffAdd(registry.dropBuffs.get(entity_other));
				registry.remove_all_components_of(entity_other);
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// generate random drop after kill enemy
// 66% have drop; in the 66% case, 40% drop size and damage increase, 10% drop beam and gravity ones
void WorldSystem::GenerateDropBuff(Entity entity)
{
	Motion& motion = registry.motions.get(entity);

	srand(time(NULL));

	int rawValue = rand() % 10;
	if (rawValue < 3) { 
		return;
	}

	int randomValue;
	if (rawValue < 7) {
		randomValue = rawValue % 2;
	}
	else if (rawValue == 7 || rawValue == 8) {
		randomValue = 2;
	}
	else {
		randomValue = 3;
	}
	TEXTURE_ASSET_ID id;

	switch (randomValue) {
		case 1:
			id = TEXTURE_ASSET_ID::DROPBALLDAMAGE;
			break;
		case 2:
			id = TEXTURE_ASSET_ID::DROPGRAVITY;
			break;
		case 3:
			id = TEXTURE_ASSET_ID::DROPBEAM;
			break;
		default:
			id = TEXTURE_ASSET_ID::DROPBALLSIZE;
			break;
	}
	Entity drop = createDropBuff(renderer, motion.position, id);
	DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
	dropBuff.id = randomValue;
	dropBuff.increaseValue = rand() % 7 + 2;
}

// handle value add when collision
void WorldSystem::DropBuffAdd(DropBuff& drop)
{
	PinBall& pinBall = registry.pinBalls.get(player);
	switch (drop.id) {
		case 0:
			pinBall.pinBallSize += drop.increaseValue;
			if (pinBall.pinBallSize > pinBall.maxPinBallSize) {
				pinBall.pinBallSize = pinBall.maxPinBallSize;
				
			}
			printf("Picked up size upgrade ");
			break;

		case 1:
			pinBall.pinBallDamage += drop.increaseValue;
			if (pinBall.pinBallDamage > pinBall.maxPinBallDamage) {
				pinBall.pinBallDamage = pinBall.maxPinBallDamage;
				
			}
			printf("Picked up damage upgrade ");
			break;

		case 2: 
			pinBall.antiGravityCount++;
			printf("Picked up antiGravity ");
			break;

		case 3:
			pinBall.tractorBeamCount++;
			printf("Picked up tractorBeam ");
			break;
		default:
			break;
	}
}