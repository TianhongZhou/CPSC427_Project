#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
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
	motion.scale = mesh.original_size * 80.f;

	registry.renderRequests.insert(
		entity,
		{ id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
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

Entity createPolygonByVertex(RenderSystem* renderer, const std::vector<vec2>& vertices, GEOMETRY_BUFFER_ID id)
{
	auto entity = Entity();
    registry.combat.emplace(entity);

    // Generate a custom mesh based on the provided vertices
	Mesh& mesh = renderer->getMesh(id);
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
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		  EFFECT_ASSET_ID::SALMON,
		  id });

	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
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
	motion.scale = mesh.original_size * 100.f;

	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

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
	motion.scale = mesh.original_size * 100.f;

	// registry.players.emplace(entity);
	registry.mainWorldEnemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			vec2(0.2, -0.5),
			vec2(-10, 48.f / 2.0f + 20) });
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

Entity createRoad(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ROAD);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 10.f;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::ROAD });

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
	motion.scale = { window_width_px, window_height_px };

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	// add things
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	// Add door
	Entity door = createDoor({ 0,0 }, { 0,0 }); //intialized below
	Motion& door_motion = registry.motions.get(door);
	float door_width = 50;
	float door_height = 60;
	door_motion.position = { w / 2.f - door_width / 2.f, door_height / 2.f };
	door_motion.scale = { door_width, door_height };
	door_motion.angle = 0;
	door_motion.velocity = { 0,0 };
	registry.colors.insert(door, { 0, 0, 0 });

	// Add spikes
	Entity spikes = createSpikes({ 100, 100 }, { 80, 80 });
	registry.colors.insert(spikes, { 0.5, 0.5, 0.5 });

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution1(100.0f, w - 100.f);
	std::uniform_real_distribution<float> distribution2(200.0f, h - 200.f);
	srand(time(NULL));
	for (int i = 0; i < 4; i++) {
		Entity spikes = createSpikes({ 100 * i, distribution2(gen) }, {80, 80});
		registry.colors.insert(spikes, { 0.5, 0.5, 0.5 });
		
		int randomValue = rand() % 2;
		TEXTURE_ASSET_ID id = TEXTURE_ASSET_ID::DROPBALLSIZE;

		if (randomValue == 1) {
			id = TEXTURE_ASSET_ID::DROPBALLDAMAGE;
		}
		Entity drop = createDropBuff(renderer, { distribution1(gen), distribution2(gen)}, id);
		DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
		dropBuff.id = randomValue;
		dropBuff.increaseValue = rand() % 7 + 2;
	}

	return entity;
}

Entity createRoom(RenderSystem* renderer, vec2 pos)
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
	motion.scale = {window_width_px, window_height_px};

	registry.rooms.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(-450.0f, 450.0f);
    std::uniform_real_distribution<float> distribution2(0.0f, 1.0f);
	Room& room = registry.rooms.get(entity);
	for (int i=0; i<2; i++) {
		room.enemies[i] = createRoomEnemy(renderer, { pos[0]+distribution1(gen), pos[1]+distribution1(gen), }, pos, 700.f, i == 0);
		registry.colors.insert(room.enemies[i], { distribution2(gen), distribution2(gen), distribution2(gen) });
	}
	PositionKeyFrame& positionKeyFrame = registry.positionKeyFrames.emplace(room.enemies[0]);
	positionKeyFrame.timeIncrement = 0.0f;
	positionKeyFrame.timeAccumulator = 0.1f;
	std::vector<vec3> keyFrames = {};
	keyFrames.push_back({ 0.0f, 600, 400 });
	keyFrames.push_back({ 10.0f, 600, 600 });
	keyFrames.push_back({ 20.0f, 400, 600 });
	keyFrames.push_back({ 30.0f, 400, 400 });
	keyFrames.push_back({ 40.0f, 600, 400 });
	positionKeyFrame.keyFrames = keyFrames;

	srand(time(NULL));

	for (int i = 0; i < 4; i++) {
		int randomValue = rand() % 2;
		TEXTURE_ASSET_ID id = TEXTURE_ASSET_ID::DROPBALLSIZE;

		if (randomValue == 1) {
			id = TEXTURE_ASSET_ID::DROPBALLDAMAGE;
		}
		Entity drop = createDropBuff(renderer, { pos[0] + distribution1(gen), pos[1] + distribution2(gen) }, id);
		DropBuff& dropBuff = registry.dropBuffs.emplace(drop);
		dropBuff.id = randomValue;
		dropBuff.increaseValue = rand() % 7 + 2;
	}

	return entity;
}

Entity createEnemyWave(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ENEMYWAVE);
    registry.combat.emplace(entity);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 50.f;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::ENEMYWAVE });

	return entity;
}


Entity createPinBallEnemyHealth(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PINBALLENEMYBLOOD);
	registry.meshPtrs.emplace(entity, &mesh);

	registry.combat.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 20.f;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::PINBALLENEMYBLOOD });

	return entity;
}

 Entity createPinBallEnemy(RenderSystem* renderer, vec2 pos, vec2 boundary, float xScale, int attackType, float attackCd)
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
	motion.scale = vec2(mesh.original_size.x * xScale , mesh.original_size.y) * 50.f;
		

	PinBallEnemy& enemy = registry.pinballEnemies.emplace(entity);
	enemy.boundary = boundary;
	enemy.maxHealth = 100.f;
	enemy.currentHealth = 100.f;
	enemy.invincibilityTimer = 0.0f;

	Entity healthBar = createPinBallEnemyHealth(renderer, { pos.x, pos.y-50 });
	registry.colors.insert(healthBar, { 0.2, 0.2, 0.2 });
	Entity healthAmortized = createPinBallEnemyHealth(renderer, { pos.x, pos.y-50 });
	registry.colors.insert(healthAmortized, { 1, 1, 1 });

	Entity health = createPinBallEnemyHealth(renderer, { pos.x, pos.y-50 });
	registry.colors.insert(health, { 1, 0, 0 });
	registry.healthBar.emplace(health);
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

Entity createBall(RenderSystem* renderer, vec2 pos, float size) 
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
	motion.scale = mesh.original_size * size * 0.8f;

	// registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::BALL });

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::PEBBLE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createPebble(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.rooms.emplace(entity); //?

	registry.balls.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROUND, // TEXTURE_COUNT indicates that no txture is needed
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
	motion.scale = size;

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
	motion.scale = size;

	registry.playerBullets.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYERBULLET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
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
	motion.scale = size;

	registry.enemyBullets.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMYBULLET, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
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
	particle.color = color;
	particle.lifespan = lifespan;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::SALMON,
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