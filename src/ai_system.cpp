// internal
#include "ai_system.hpp"
#include "world_init.hpp"
#define M_PI 3.14159265358979323846 /* pi */
#define ENEMY_VERSION_WIDTH M_PI / 9
#define ENEMY_VERSION_LENGTH 400.f

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
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

	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		if (registry.mainWorldEnemies.has(motion_container.entities[i]))
		{
			Motion &enemyMotion = motion_container.components[i];
			Enemy &enemy = registry.mainWorldEnemies.get(motion_container.entities[i]);
			float angleToPlayer = atan2(playerMotion.position.y - enemyMotion.position.y, playerMotion.position.x - enemyMotion.position.x);
			float distanceToPlayer = glm::length(playerMotion.position - enemyMotion.position);
			if (enemy.seePlayer)
			{
				// Enemy chasing player
				enemyMotion.angle = angleToPlayer;
			}
			else
			{
				if (enemy.keyFrame) 
				{
					PositionKeyFrame& positionKeyFrame = registry.positionKeyFrames.get(motion_container.entities[i]);

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
					if (xDiff > enemy.roomScale * 0.4) {
						enemyMotion.angle = 0;
					}
					else if (-xDiff > enemy.roomScale * 0.4) {
						enemyMotion.angle = M_PI;
					}
					if (yDiff > enemy.roomScale * 0.4) {
						enemyMotion.angle = M_PI / 2;
					}
					else if (-yDiff > enemy.roomScale * 0.4) {
						enemyMotion.angle = -M_PI / 2;
					}
					// Randomly move in room
					if (enemy.randomMoveTimer <= 0.0f)
					{
						if (enemy.haltTimer <= 0.0f)
						{
							enemyMotion.velocity.x = 50.f;
							float ran = (float)(rand() % 4 - 1);
							float randomAngle = ran * M_PI / 2;
							enemyMotion.angle = randomAngle;
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
				if (distanceToPlayer <= ENEMY_VERSION_LENGTH && enemyMotion.angle + ENEMY_VERSION_WIDTH > angleToPlayer && enemyMotion.angle - ENEMY_VERSION_WIDTH < angleToPlayer)
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

	for (int i = 0; i < registry.mainWorldEnemies.entities.size(); i++) {
		Entity entity = registry.mainWorldEnemies.entities[i];
		Enemy& enemy = registry.mainWorldEnemies.get(entity);

		// If the player is within half the enemy's field of view, enemy attack
		if (enemy.seePlayer == true) {

			bool temp = false;
			if (registry.spriteSheets.has(entity))
			{
				SpriteSheet& spriteSheet = registry.spriteSheets.get(entity);
				if (spriteSheet.next_sprite == TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET)
				{
					continue;
				}
				temp = spriteSheet.xFlip;
			}
			SpriteSheet& spriteSheet = registry.spriteSheets.emplace_with_duplicates(entity);
			spriteSheet.next_sprite = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
			spriteSheet.frameIncrement = 0.04f;
			spriteSheet.frameAccumulator = 0.0f;
			spriteSheet.spriteSheetHeight = 1;
			spriteSheet.spriteSheetWidth = 9;
			spriteSheet.totalFrames = 9;
			spriteSheet.origin = TEXTURE_ASSET_ID::ENEMYWALKSPRITESHEET;
			spriteSheet.loop = true;
			spriteSheet.xFlip = temp;

			RenderRequest& renderRequest = registry.renderRequests.get(entity);
			renderRequest.used_texture = TEXTURE_ASSET_ID::ENEMYATTACKSPRITESHEET;
		}
	}
}