// internal
#include "ai_system.hpp"
#include "world_init.hpp"
#include "world_system.hpp"

#define M_PI 3.14159265358979323846 /* pi */
#define ENEMY_VERSION_WIDTH M_PI / 9
#define ENEMY_VERSION_LENGTH 400.f

void AISystem::step(float elapsed_ms)
{
	float step_seconds = elapsed_ms / 1000.f;
	auto &motion_container = registry.motions;

	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		if (registry.pinballEnemies.has(motion_container.entities[i]))
		{
			Motion &enemyMotion = motion_container.components[i];
			PinBallEnemy &enemy = registry.pinballEnemies.get(motion_container.entities[i]);
        	physObj& enemyPhys = registry.physObjs.get(motion_container.entities[i]);

			// Periodically veritical move
			// for (int j=0; j<enemyPhys.VertexCount; j++) {
            // 	enemyPhys.Vertices[j].pos.y += 10.f*sin(step_seconds);
            // 	enemyPhys.Vertices[j].oldPos.y += 10.f*sin(step_seconds);
			// }

			// Turn to another direction if near the boundary
			if (enemyMotion.position.x < enemy.boundary.x)
			{
				for (int j=0; j<enemyPhys.VertexCount; j++) {
					enemyPhys.Vertices[j].accel.x = 0.01f;
				}
			}
			else if (enemyMotion.position.x > enemy.boundary.y)
			{
				for (int j=0; j<enemyPhys.VertexCount; j++) {
					enemyPhys.Vertices[j].accel.x = -0.01f;
				}
			}

			// Random horizontally move in combat scene
			if (enemy.randomMoveTimer <= 0.0f)
			{
				int ran = rand() % 2;
				for (int j=0; j<enemyPhys.VertexCount; j++) {
					enemyPhys.Vertices[j].accel.x = 0.01f*(ran == 0 ? -1 : 1);
				}
				enemy.randomMoveTimer = 3.f + rand() % 3;
			}
			else
			{
				enemy.randomMoveTimer -= step_seconds;
			}

			// Health bar follows enemy
			for (int j=0; j<enemy.healthBar.size(); j++) {
				if (motion_container.has(enemy.healthBar[j])) {
					Motion& healthbarMotion = motion_container.get(enemy.healthBar[j]);
					healthbarMotion.position.x = enemyMotion.position.x;
					healthbarMotion.position.y = enemyMotion.position.y-50.f;
				}
			}

			// Update healthbar
			if (motion_container.has(enemy.healthBar[1]) && motion_container.has(enemy.healthBar[2])) {
				Motion& barMotion = motion_container.get(enemy.healthBar[0]);
				Motion& healthMotion = motion_container.get(enemy.healthBar[1]);
				Motion& amortizedMotion = motion_container.get(enemy.healthBar[2]);
				healthMotion.scale.x =  barMotion.scale.x * enemy.currentHealth/enemy.maxHealth;
				healthMotion.position.x = barMotion.position.x - (barMotion.scale.x - healthMotion.scale.x) / 2;
				if (amortizedMotion.scale.x>healthMotion.scale.x) {
					amortizedMotion.scale.x -= 0.5f;
				}
				amortizedMotion.position.x = barMotion.position.x - (barMotion.scale.x - amortizedMotion.scale.x) / 2;
			}
			
		}
	}
}

void AISystem::step_world(float elapsed_ms)
{
	Motion playerMotion;
	float step_seconds = elapsed_ms / 1000.f;
	auto &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Entity entity = motion_container.entities[i];
		if (registry.players.has(entity))
		{
			playerMotion = motion_container.components[i];
		}
	}
	

	// Enemy fire rate
	bullet_spawn_timer -= elapsed_ms;

	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{	
		if (registry.snipers.has(motion_container.entities[i])) {

			Motion& enemyMotion = motion_container.components[i];
			Sniper& enemy = registry.snipers.get(motion_container.entities[i]);
			float angleToPlayer = atan2(playerMotion.position.y - enemyMotion.position.y, playerMotion.position.x - enemyMotion.position.x);

			//shoot player

			if (bullet_spawn_timer < 0 && !registry.positionKeyFrames.has(motion_container.entities[i])) {

				// Create enemy bullet
				Entity entity = createEnemyBullet({ 0,0 }, { 0,0 }); //intialized below

				Motion& motion = registry.motions.get(entity);
				motion.position = enemyMotion.position;

				float radius = 30; //* (uniform_dist(rng) + 0.3f);
				motion.scale = { radius, radius };
				motion.angle = angleToPlayer;
				motion.velocity = vec2(200.f, 0.f);
				registry.colors.insert(entity, { 1, 1, 1 });
			}

		}
		else if (registry.zombies.has(motion_container.entities[i])) {

			Motion& enemyMotion = motion_container.components[i];
			Zombie& enemy = registry.zombies.get(motion_container.entities[i]);
			float angleToPlayer = atan2(playerMotion.position.y - enemyMotion.position.y, playerMotion.position.x - enemyMotion.position.x);
			// Enemy chasing player
			enemyMotion.velocity.x = 100.f * cos(angleToPlayer);
			enemyMotion.velocity.y = 100.f * sin(angleToPlayer);
		}
		else if (registry.mainWorldEnemies.has(motion_container.entities[i]))
		{
			Motion &enemyMotion = motion_container.components[i];
			Enemy &enemy = registry.mainWorldEnemies.get(motion_container.entities[i]);
			float angleToPlayer = atan2(playerMotion.position.y - enemyMotion.position.y, playerMotion.position.x - enemyMotion.position.x);

			//shoot player

			if (bullet_spawn_timer < 0 && !registry.positionKeyFrames.has(motion_container.entities[i])) {

				// Create enemy bullet
				Entity entity = createEnemyBullet({ 0,0 }, { 0,0 }); //intialized below

				Motion& motion = registry.motions.get(entity);
				motion.position = enemyMotion.position;

				float radius = 30; //* (uniform_dist(rng) + 0.3f);
				motion.scale = { radius, radius };
				motion.angle = angleToPlayer; 
				motion.velocity = vec2(200.f, 0.f);
				registry.colors.insert(entity, { 1, 1, 1 });
			}

			float distanceToPlayer = glm::length(playerMotion.position - enemyMotion.position);
			if (enemy.seePlayer && !registry.positionKeyFrames.has(motion_container.entities[i]))
			{
				// Enemy chasing player
				// enemyMotion.angle = angleToPlayer;
				enemyMotion.velocity.x = 100.f * cos(angleToPlayer);
				enemyMotion.velocity.y = 100.f * sin(angleToPlayer);
			}
			else
			{
				if (enemy.keyFrame)
				{
					PositionKeyFrame &positionKeyFrame = registry.positionKeyFrames.get(motion_container.entities[i]);

					for (int j = 0; j < positionKeyFrame.keyFrames.size() - 1; j++)
					{
						if (positionKeyFrame.keyFrames[j].x == positionKeyFrame.timeIncrement)
						{
							enemyMotion.position = vec2(positionKeyFrame.keyFrames[j].y, positionKeyFrame.keyFrames[j].z);
							break;
						}

						if ((positionKeyFrame.keyFrames[j].x < positionKeyFrame.timeIncrement) &&
							(positionKeyFrame.keyFrames[j + 1].x > positionKeyFrame.timeIncrement))
						{
							vec2 target = vec2(positionKeyFrame.keyFrames[j + 1].y, positionKeyFrame.keyFrames[j + 1].z);
							SpriteSheet& spriteSheet = registry.spriteSheets.get(motion_container.entities[i]);
							if (target.x < enemyMotion.position.x) {
								spriteSheet.xFlip = 1;
							}
							else {
								spriteSheet.xFlip = 0;
							}
							float t = (positionKeyFrame.timeIncrement - positionKeyFrame.keyFrames[j].x) /
									  (positionKeyFrame.keyFrames[j + 1].x - positionKeyFrame.keyFrames[j].x);
							enemyMotion.position = (1.0f - t) * vec2(positionKeyFrame.keyFrames[j].y, positionKeyFrame.keyFrames[j].z) +
												   t * vec2(positionKeyFrame.keyFrames[j + 1].y, positionKeyFrame.keyFrames[j + 1].z);
							break;
						}
					}

					positionKeyFrame.timeIncrement += positionKeyFrame.timeAccumulator;
					if (positionKeyFrame.timeIncrement > positionKeyFrame.keyFrames[positionKeyFrame.keyFrames.size() - 1].x)
					{
						positionKeyFrame.timeIncrement = 0;
					}
				}
				else
				{
					// Turn to another direction if near the boundary
					float xDiff = enemy.roomPositon.x - enemyMotion.position.x;
					float yDiff = enemy.roomPositon.y - enemyMotion.position.y;
					if (xDiff > enemy.roomScale * 0.4)
					{
						enemyMotion.velocity.x = abs(enemyMotion.velocity.x);
					}
					else if (-xDiff > enemy.roomScale * 0.4)
					{
						enemyMotion.velocity.x = -abs(enemyMotion.velocity.x);
					}
					if (yDiff > enemy.roomScale * 0.4)
					{
						enemyMotion.velocity.y = abs(enemyMotion.velocity.y);
					}
					else if (-yDiff > enemy.roomScale * 0.4)
					{
						enemyMotion.velocity.y = -abs(enemyMotion.velocity.y);
					}
					// Randomly move in room
					if (enemy.randomMoveTimer <= 0.0f)
					{
						if (enemy.haltTimer <= 0.0f)
						{
							float ran = (float)(rand() % 4 - 1);
							float randomAngle = ran * M_PI / 2;
							enemyMotion.velocity.x = 50.f * cos(randomAngle);
							enemyMotion.velocity.y = 50.f * sin(randomAngle);
							enemy.randomMoveTimer = 3.f + rand() % 3;
							enemy.haltTimer = 0.3f;
						}
						else
						{
							enemyMotion.velocity.x = 0.f;
							enemy.haltTimer -= step_seconds;
						}
					}
					else
					{
						enemy.randomMoveTimer -= step_seconds;
					}
				}
				// Check if the player is within the enemy's field of view
				float enemyDirection = atan2(enemyMotion.velocity.y, enemyMotion.velocity.x);
				if (distanceToPlayer <= ENEMY_VERSION_LENGTH && enemyDirection + ENEMY_VERSION_WIDTH > angleToPlayer && enemyDirection - ENEMY_VERSION_WIDTH < angleToPlayer)
				{
					if (!registry.highLightEnemies.has(motion_container.entities[i]))
					{
						registry.highLightEnemies.emplace(motion_container.entities[i]);
						enemyMotion.velocity.x = 0.f;
						enemy.seePlayer = true;
					}
				}
			}
		}
	}

	for (int i = 0; i < registry.mainWorldEnemies.entities.size(); i++)
	{
		Entity entity = registry.mainWorldEnemies.entities[i];
		Enemy &enemy = registry.mainWorldEnemies.get(entity);

		// If the player is within half the enemy's field of view, enemy attack
		if (enemy.seePlayer == true)
		{

			bool temp = false;
			if (registry.spriteSheets.has(entity))
			{
				SpriteSheet &spriteSheet = registry.spriteSheets.get(entity);
				if (spriteSheet.next_sprite == TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET)
				{
					continue;
				}
				temp = spriteSheet.xFlip;
			}
			SpriteSheet &spriteSheet = registry.spriteSheets.emplace_with_duplicates(entity);
			spriteSheet.next_sprite = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
			spriteSheet.frameIncrement = 0.04f;
			spriteSheet.frameAccumulator = 0.0f;
			spriteSheet.spriteSheetHeight = 1;
			spriteSheet.spriteSheetWidth = 9;
			spriteSheet.totalFrames = 9;
			spriteSheet.origin = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
			spriteSheet.loop = true;
			spriteSheet.xFlip = temp;

			RenderRequest &renderRequest = registry.renderRequests.get(entity);
			renderRequest.used_texture = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
		}
	}
	if (bullet_spawn_timer < 0) {
		bullet_spawn_timer = 3000.f;
	}
}