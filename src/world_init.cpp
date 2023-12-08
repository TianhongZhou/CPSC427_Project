#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"
#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime> 

Entity createDropBuff(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID id)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 50.f;
	registry.renderRequests.insert(
		entity,
		{ id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			id,
			vec2(0.f, -0.5f),
			vec2(0.f, 0.f)});

	return entity;
}

Entity createShadow(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SHADOW,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPinballWall(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::RECT);
    registry.combat.emplace(entity);

    // Generate a custom mesh based on the provided vertices
	mesh.fromVertices(vertices,
		mesh.vertices,
		mesh.vertex_indices,
		mesh.original_size);

	renderer->bindVBOandIBO(id,
		mesh.vertices,
		mesh.vertex_indices);
	registry.meshPtrs.emplace(entity, &mesh);

	// Assuming the position to be the centroid of the polygon for motion purposes
	vec2 centroid = std::accumulate(vertices.begin(), vertices.end(), vec2(0, 0)) / float(vertices.size());
	Motion& motion = registry.motions.emplace(entity);
	motion.position = centroid;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WALL,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPinballFlipper(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::RECT);
	registry.combat.emplace(entity);

	// Generate a custom mesh based on the provided vertices
	mesh.fromVertices(vertices,
		mesh.vertices,
		mesh.vertex_indices,
		mesh.original_size);

	renderer->bindVBOandIBO(id,
		mesh.vertices,
		mesh.vertex_indices);
	registry.meshPtrs.emplace(entity, &mesh);

	// Assuming the position to be the centroid of the polygon for motion purposes
	vec2 centroid = std::accumulate(vertices.begin(), vertices.end(), vec2(0, 0)) / float(vertices.size());
	Motion& motion = registry.motions.emplace(entity);
	motion.position = centroid;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FLIPPER,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos, float currentHealth)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
    registry.mainWorld.emplace(entity);
	registry.pinBalls.emplace(entity);

    // Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 75.f;

	Player& player = registry.players.emplace(entity);
	player.currentHealth = currentHealth;

	Entity healthBar = createHealth(renderer, { pos.x, pos.y-50 }, false);
	registry.colors.insert(healthBar, { 0.2, 0.2, 0.2 });
	Entity healthAmortized = createHealth(renderer, { pos.x, pos.y-50 }, false);
	registry.colors.insert(healthAmortized, { 1, 1, 1 });

	Entity health = createHealth(renderer, { pos.x, pos.y-50 }, false);
	registry.colors.insert(health, { 1, 0, 0 });
	player.healthBar[0] = healthBar;
	player.healthBar[1] = health;
	player.healthBar[2] = healthAmortized;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.2, -0.5),
			vec2(-10, 48.f / 2.0f + 8) });

	return entity;
}

// Room Generation 

Entity createRoomEnemy(RenderSystem* renderer, vec2 pos, vec2 roomPostion, float roomScale, bool keyFrame)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = vec2(50.f,0.f);
	motion.scale = mesh.original_size * 65.f;

	// registry.players.emplace(entity);
	registry.mainWorldEnemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.2, -0.5),
			vec2(-10, 48.f / 2.0f + 8) });
	Enemy& ene = registry.mainWorldEnemies.get(entity);
	ene.roomPositon = roomPostion;
	ene.roomScale = roomScale;
	ene.keyFrame = keyFrame;

	SpriteSheet& spriteSheet = registry.spriteSheets.emplace(entity);
	spriteSheet.next_sprite = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
	spriteSheet.frameIncrement = 0.04f;
	spriteSheet.frameAccumulator = 0.0f;
	spriteSheet.spriteSheetHeight = 1;
	spriteSheet.spriteSheetWidth = 6;
	spriteSheet.totalFrames = 6;
	spriteSheet.origin = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
	spriteSheet.loop = true;
	if (motion.velocity.x < 0.f)
	{
		spriteSheet.xFlip = true;
	}
	/*RenderRequest& renderRequest = registry.renderRequests.get(player);
	renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET;*/

	return entity;
}

Entity createRoomSniper(RenderSystem* renderer, vec2 pos, vec2 roomPostion, float roomScale, bool keyFrame)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = vec2(0.f, 0.f);
	motion.scale = mesh.original_size * 65.f;

	// registry.players.emplace(entity);
	registry.mainWorldEnemies.emplace(entity);
	registry.snipers.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.2, -0.5),
			vec2(-10, 48.f / 2.0f + 8) });

	SpriteSheet& spriteSheet = registry.spriteSheets.emplace(entity);
	spriteSheet.next_sprite = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
	spriteSheet.frameIncrement = 0.04f;
	spriteSheet.frameAccumulator = 0.0f;
	spriteSheet.spriteSheetHeight = 1;
	spriteSheet.spriteSheetWidth = 9;
	spriteSheet.totalFrames = 9;
	spriteSheet.origin = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
	spriteSheet.loop = true;
	if (motion.velocity.x < 0.f)
	{
		spriteSheet.xFlip = true;
	}
	return entity;
}

Entity createRoomZombie(RenderSystem* renderer, vec2 pos, vec2 roomPostion, float roomScale, bool keyFrame)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = vec2(50.f, 0.f);
	motion.scale = mesh.original_size * 65.f;

	// registry.players.emplace(entity);
	registry.mainWorldEnemies.emplace(entity);
	registry.zombies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.2, -0.5),
			vec2(-10, 48.f / 2.0f + 8) });

	SpriteSheet& spriteSheet = registry.spriteSheets.emplace(entity);
	spriteSheet.next_sprite = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
	spriteSheet.frameIncrement = 0.04f;
	spriteSheet.frameAccumulator = 0.0f;
	spriteSheet.spriteSheetHeight = 1;
	spriteSheet.spriteSheetWidth = 6;
	spriteSheet.totalFrames = 6;
	spriteSheet.origin = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
	spriteSheet.loop = true;
	if (motion.velocity.x < 0.f)
	{
		spriteSheet.xFlip = true;
	}
	return entity;
}

Entity createRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window, int room_num)
{
	auto entity = Entity();
	switch (room_num) {
	case -1:
		return createEmptyRoom(renderer, pos, window);
		break;
	case 0:
		return createStartingRoom(renderer, pos, window);
		break;
	case 1:
		return createRoom1(renderer, pos);
		break;
	/*case 2:
		return createMaze1(renderer, pos);
		break;*/
	case 2:
		return createRoom2(renderer, pos);
		break;
	case 3:
		return createRoom3(renderer, pos);
		break;
	}

	return entity; //This should never happen

}


//Entity createMaze(RenderSystem* renderer, vec2 pos, GLFWwindow* window)
//{
//	auto entity = Entity();
//	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//	registry.meshPtrs.emplace(entity, &mesh);
//	registry.mainWorld.emplace(entity);
//
//	// Setting initial motion values
//	Motion& motion = registry.motions.emplace(entity);
//	motion.position = pos;
//	motion.angle = 0.f;
//	motion.velocity = { 0.f, 0.f };
//
//	int w, h;
//	glfwGetWindowSize(window, &w, &h);
//	std::cout << "MAZE window width, height: " << w << " " << h << std::endl;
//
//	motion.scale = { window_width_px, window_height_px };
//
//	registry.rooms.emplace(entity);
//
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::GROUND,
//			EFFECT_ASSET_ID::TEXTURED,
//			GEOMETRY_BUFFER_ID::SPRITE });
//
//	float spike_size = 80;
//	int horizontal = int( (window_width_px - 100) / spike_size) - 1;
//
//	for (int i = 0; i < horizontal; i++) {
//		Entity spikes = createSpikes({ i * spike_size + 100, 200}, { spike_size, spike_size });
//		registry.colors.insert(spikes, { 0.5, 0.5, 0.5 });
//
//		if (i == horizontal / 3 || i == 2 * horizontal / 3) {
//			Entity drop = createDropBuff(renderer, { i * spike_size + 100, 100 }, TEXTURE_ASSET_ID::DROPBALLSIZE);
//			DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
//
//			Entity drop2 = createDropBuff(renderer, { i * spike_size + 100, 300 }, TEXTURE_ASSET_ID::DROPBALLDAMAGE);
//			DropBuff& dropBuff2 = registry.dropBuffs.emplace(drop2);
//		}
//
//		Entity spikes2 = createSpikes({ window_width_px - i * spike_size - 100, 400 }, { spike_size, spike_size });
//		registry.colors.insert(spikes2, { 0.5, 0.5, 0.5 });
//	}
//
//	return entity;
//}

Entity createBar(RenderSystem* renderer, vec2 pos, vec2 scale) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ROAD);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * scale;

	registry.mazes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::ROAD });

	return entity;
}

Entity createMaze1(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	// Add door
	Entity door = createDoor({ 0,0 }, { 0,0 }); //intialized below
	Motion& door_motion = registry.motions.get(door);
	float door_width = 50;
	float door_height = 60;
	door_motion.position = { window_width_px / 2.f - door_width / 2.f, door_height / 2.f };
	door_motion.scale = { door_width, door_height };
	door_motion.angle = 0;
	door_motion.velocity = { 0,0 };
	registry.colors.insert(door, { 0, 0, 0 });

	createBar(renderer, { 250.f, 250.f }, { 50.f, 50.f });

	return entity;
}

Entity createEmptyRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createStartingRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	//printf("This is Starting Room size: %d, %d\n", )
	motion.scale = { window_width_px, window_height_px };
	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::NORMAL,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::GROUNDNORMAL });

	// add things
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	printf("This is window size in starting room: %d, %d\n", w, h);

	// Add door
	Entity door = createDoor({ 0,0 }, { 0,0 }); //intialized below
	Motion& door_motion = registry.motions.get(door);
	float door_width = 50;
	float door_height = 60;
	door_motion.position = { window_width_px / 2.f - door_width / 2.f, door_height / 2.f };
	door_motion.scale = { door_width, door_height };
	door_motion.angle = 0;
	door_motion.velocity = { 0,0 };
	registry.colors.insert(door, { 0, 0, 0 });

	// Add spikes
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(100.0f, 4 * window_width_px / 5);
	std::uniform_real_distribution<float> distribution2(100.0f, 4 * window_height_px / 5);
	srand(time(NULL));

	//for (int i = 0; i < 4; i++) {
	//	Entity spikes = createSpikes({ distribution1(gen), distribution2(gen)}, {80, 80});
	//	//Entity spikes = createSpikes({ 100 * i, distribution2(gen) }, {80, 80});
	//	registry.colors.insert(spikes, { 0.5, 0.5, 0.5 });
	//}

	float spike_size = 80;
	int horizontal = int((window_width_px - 100) / spike_size) - 1;

	for (int i = 0; i < horizontal; i++) {
		Entity spikes = createSpikes({ i * spike_size + 100, 200 }, { spike_size, spike_size });
		registry.colors.insert(spikes, { 0.5, 0.5, 0.5 });

		Entity spikes2 = createSpikes({ window_width_px - i * spike_size - 100, 400 }, { spike_size, spike_size });
		registry.colors.insert(spikes2, { 0.5, 0.5, 0.5 });

		if (i == horizontal / 3 || i == 2 * horizontal / 3) {
			Entity drop = createDropBuff(renderer, { i * spike_size + 100, 100 }, TEXTURE_ASSET_ID::DROPBALLSIZE);
			DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
			dropBuff.increaseValue = 2;

			Entity drop2 = createDropBuff(renderer, { i * spike_size + 100, 300 }, TEXTURE_ASSET_ID::DROPBALLDAMAGE);
			DropBuff& dropBuff2 = registry.dropBuffs.emplace(drop2);
			dropBuff2.increaseValue = 2;
		}	
	}

	return entity;
}

Entity createRoom1(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::GROUNDNORMAL });

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(-250.0f, 250.0f);
	std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);
	Room& room = registry.rooms.get(entity);
	for (int i = 0; i < 3; i++) {
		room.enemies[i] = createRoomSniper(renderer, { pos[0] + distribution1(gen), pos[1] + distribution1(gen), }, pos, 700.f, i == 0);
		registry.colors.insert(room.enemies[i], { distribution2(gen), distribution2(gen), distribution2(gen) });
	}

	registry.bosses.emplace(room.enemies[0]);
	Motion& motion2 = registry.motions.get(room.enemies[0]);
	motion2.scale *= 1.8f;
	registry.lights.emplace(room.enemies[0]);

	return entity;
}

Entity createRoom2(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::GROUNDNORMAL });

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(-250.0f, 250.0f);
	std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);
	Room& room = registry.rooms.get(entity);
	for (int i = 0; i < 3; i++) {
		room.enemies[i] = createRoomZombie(renderer, { pos[0] + distribution1(gen), pos[1] + distribution1(gen), }, pos, 700.f, i == 0);
		registry.colors.insert(room.enemies[i], { distribution2(gen), distribution2(gen), distribution2(gen) });
	}

	registry.bosses.emplace(room.enemies[0]);
	Motion& motion2 = registry.motions.get(room.enemies[0]);
	motion2.scale *= 1.8f;
	registry.lights.emplace(room.enemies[0]);

	return entity;
}



Entity createRoom3(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.mainWorld.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	/*int w, h;
	glfwGetWindowSize(window, &w, &h);*/
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::GROUNDNORMAL });

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(-250.0f, 250.0f);
	std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);
	Room& room = registry.rooms.get(entity);
	for (int i = 0; i < 3; i++) {
		room.enemies[i] = createRoomEnemy(renderer, { pos[0] + distribution1(gen), pos[1] + distribution1(gen), }, pos, 700.f, i == 0);
		registry.colors.insert(room.enemies[i], { distribution2(gen), distribution2(gen), distribution2(gen) });
	}

	registry.bosses.emplace(room.enemies[0]);
	Motion&  motion2 = registry.motions.get(room.enemies[0]);
	motion2.scale *= 1.8f;
	registry.lights.emplace(room.enemies[0]);
	PositionKeyFrame& positionKeyFrame = registry.positionKeyFrames.emplace(room.enemies[0]);
	positionKeyFrame.timeIncrement = 0.0f;
	positionKeyFrame.timeAccumulator = 0.1f;
	std::vector<vec3> keyFrames = {};
	keyFrames.push_back({ 0.0f, window_width_px / 2 - 120, 50 });
	keyFrames.push_back({ 20.0f, window_width_px / 2 + 120, 50 });
	keyFrames.push_back({ 40.0f, window_width_px / 2 - 120, 50 });
	positionKeyFrame.keyFrames = keyFrames;

	return entity;
}

Entity* createPinballRoom(RenderSystem* renderer, vec2 pos, GLFWwindow* window)
{
	auto entities = new Entity[8];
	TEXTURE_ASSET_ID textures[8] = {TEXTURE_ASSET_ID::PINBALLBACKGROUND1, TEXTURE_ASSET_ID::PINBALLBACKGROUND2, TEXTURE_ASSET_ID::PINBALLBACKGROUND3, TEXTURE_ASSET_ID::PINBALLBACKGROUND4, TEXTURE_ASSET_ID::PINBALLBACKGROUND5, TEXTURE_ASSET_ID::PINBALLBACKGROUND6, TEXTURE_ASSET_ID::PINBALLBACKGROUND7, TEXTURE_ASSET_ID::PINBALLBACKGROUND8};
	for (int i=0; i<8; i++) {
		auto entity = Entity();
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);
		registry.combat.emplace(entity);

		// Setting initial motion values
		Motion& motion = registry.motions.emplace(entity);
		motion.position = pos;
		motion.angle = 0.f;
		motion.velocity = { 0.f, 0.f };

		/*int w, h;
		glfwGetWindowSize(window, &w, &h);*/
		//printf("This is Starting Room size: %d, %d\n", )
		motion.scale = { window_width_px, window_height_px };

		registry.paras.emplace(entity);

		registry.renderRequests.insert(
			entity,
			{ textures[i],
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
		entities[i] = entity;
	}

	return entities;
}

Entity createHealth(RenderSystem* renderer, vec2 pos, bool combat)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PINBALLENEMYBLOOD);
	registry.meshPtrs.emplace(entity, &mesh);

	if (combat) registry.combat.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 20.f;
	registry.healthBar.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::PINBALLENEMYBLOOD });

	return entity;
}

Entity createSwarmEnemy(RenderSystem* renderer, vec2 pos)
{
    float scale = 8.f;
    auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SWARMENEMY);
    registry.meshPtrs.emplace(entity, &mesh);

    registry.combat.emplace(entity);

    // Setting initial motion values
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.scale = vec2(mesh.original_size.x, mesh.original_size.y) * scale;

    SwarmEnemy& enemy = registry.swarmEnemies.emplace(entity);
    enemy.invincibilityTimer = 0.0f;

    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::TEXTURE_COUNT,
              EFFECT_ASSET_ID::SALMON,
              GEOMETRY_BUFFER_ID::SWARMENEMY});
//    createNewRectangleTiedToEntity(entity, mesh.original_size.x * scale, mesh.original_size.y*scale, registry.motions.get(entity).position, true, 1.0);
//
//    physObj& physObj = registry.physObjs.get(entity);
//    physObj.hasGravity = false;
    return entity;

}


 Entity createPinBallEnemy(RenderSystem *renderer, vec2 pos, vec2 boundary, float xScale, int attackType, float attackCd,
                    float yScale)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PINBALLENEMY);
	registry.meshPtrs.emplace(entity, &mesh);

    registry.combat.emplace(entity);

    // Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(mesh.original_size.x * xScale , mesh.original_size.y * yScale) * 50.f;
		

	PinBallEnemy& enemy = registry.pinballEnemies.emplace(entity);
	enemy.boundary = boundary;
	enemy.maxHealth = 100.f;
	enemy.currentHealth = 100.f;
	enemy.invincibilityTimer = 0.0f;

	Entity healthBar = createHealth(renderer, { pos.x, pos.y-50 }, true);
	registry.colors.insert(healthBar, { 0.2, 0.2, 0.2 });
	Entity healthAmortized = createHealth(renderer, { pos.x, pos.y-50 }, true);
	registry.colors.insert(healthAmortized, { 1, 1, 1 });

	Entity health = createHealth(renderer, { pos.x, pos.y-50 }, true);
	registry.colors.insert(health, { 1, 0, 0 });
	enemy.healthBar[0] = healthBar;
	enemy.healthBar[1] = health;
	enemy.healthBar[2] = healthAmortized;

	enemy.attackType = attackType;
	enemy.attackCooldown = attackCd;
	enemy.attackTimer = attackCd;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::OCT });
	createNewRectangleTiedToEntity(entity, mesh.original_size.x * xScale*50.f, mesh.original_size.y*50.f, registry.motions.get(entity).position, false, 1.0);

	return entity;
}

Entity createBall(RenderSystem* renderer, vec2 pos, float size, float trail) 
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BALL);
	registry.meshPtrs.emplace(entity, &mesh);

    registry.combat.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * size * 0.7f;

	Ball& ball = registry.balls.emplace(entity);
	ball.trail = trail;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PINBALL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createDoor(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	registry.doors.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND, // TODO: Using blacked out ground for now, change this
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createSpikes(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size * 0.75f;

	registry.spikes.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FISH, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPlayerBullet(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size * 0.7f;

	registry.playerBullets.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYERBULLET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.f, 0.f),
			vec2(-10, 48.f / 2.0f + 20) });

	return entity;
}

Entity createEnemyBullet(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size * 0.7f;

	registry.enemyBullets.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYBULLET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			vec2(0.f, 0.f),
			vec2(-10, 48.f / 2.0f + 20) });

	return entity;
}


Entity createParticle(RenderSystem* renderer, vec2 pos, float size, vec2 vel, vec3 color, float lifespan)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BALL);
	registry.meshPtrs.emplace(entity, &mesh);

    registry.combat.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = vel;
	motion.scale = mesh.original_size * size;

	Particle& particle = registry.particles.emplace(entity);
	particle.lifespan = lifespan;

    for (int i=0; i<8; i++) {
		
	}
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PEBBLE,
			GEOMETRY_BUFFER_ID::BALL });
	registry.colors.insert(entity, color);

	return entity;
}


void createNewRectangleTiedToEntity(Entity e, float w, float h, vec2 centerPos, bool moveable, float knockbackCoef) {


	//	auto& entity = Entity();

	Vertex_Phys newV{};
	registry.physObjs.emplace(e);
	physObj test0 = registry.physObjs.components[0];


	//	0-----1
	//	|	  |
	//	3-----2

	physObj& newObj = registry.physObjs.get(e);

	newObj.moveable = moveable;
	newObj.knockbackCoef = knockbackCoef;

	newV.pos = vec2(centerPos.x - w / 2, centerPos.y + h / 2);
	newV.oldPos = vec2(centerPos.x - w / 2, centerPos.y + h / 2);
	newV.accel = vec2(0.0, 0.0);


	newObj.Vertices[0] = newV;

	newV.pos = vec2(centerPos.x + w / 2, centerPos.y + h / 2);
	newV.oldPos = vec2(centerPos.x + w / 2, centerPos.y + h / 2);

	newObj.Vertices[1] = newV;

	newV.pos = vec2(centerPos.x - w / 2, centerPos.y - h / 2);
	newV.oldPos = vec2(centerPos.x - w / 2, centerPos.y - h / 2);

	newObj.Vertices[3] = newV;

	newV.pos = vec2(centerPos.x + w / 2, centerPos.y - h / 2);
	newV.oldPos = vec2(centerPos.x + w / 2, centerPos.y - h / 2);


	newObj.Vertices[2] = newV;

	newObj.VertexCount = 4;

	physObj test1 = registry.physObjs.components[0];

	Edge newEdge{};



	newObj.Edges[0].v1 = 0;
	newObj.Edges[0].v2 = 1;
	newObj.Edges[0].len = w;


	newObj.Edges[1].v1 = 1;
	newObj.Edges[1].v2 = 2;
	newObj.Edges[1].len = h;



	newObj.Edges[2].v1 = 2;
	newObj.Edges[2].v2 = 3;
	newObj.Edges[2].len = w;

	physObj test2 = registry.physObjs.components[0];


	newObj.Edges[3].v1 = 3;
	newObj.Edges[3].v2 = 0;
	newObj.Edges[3].len = h;



	newObj.Edges[4].v1 = 0;
	newObj.Edges[4].v2 = 2;
	newObj.Edges[4].len = sqrt(h * h + w * w);





	newObj.EdgesCount = 5;

	newObj.center = centerPos;

}
