// internal
#include "ai_system.hpp"
#include "world_init.hpp"
# define M_PI           3.14159265358979323846  /* pi */
# define ENEMY_VERSION_WIDTH  M_PI/9
# define ENEMY_VERSION_LENGTH  400.f

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
}

void AISystem::step_world(float elapsed_ms)
{
	Motion playerMotion;
	float step_seconds = elapsed_ms / 1000.f;
	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Entity entity = motion_container.entities[i];
		if (registry.players.has(entity)) {
			playerMotion = motion_container.components[i];
		}
	}

	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i) {
		if (registry.mainWorldEnemies.has(motion_container.entities[i])) {
			Motion& enemyMotion = motion_container.components[i];
			Enemy& enemy = registry.mainWorldEnemies.get(motion_container.entities[i]);
			float angleToPlayer = atan2(playerMotion.position.y - enemyMotion.position.y, playerMotion.position.x - enemyMotion.position.x);
			float distanceToPlayer = glm::length(playerMotion.position-enemyMotion.position);
			if (enemy.seePlayer) {
				//Enemy chasing player
				enemyMotion.angle = angleToPlayer;
			} else {
				// Randomly move in room
                if (enemy.randomMoveTimer <= 0.0f) {
					if (enemy.haltTimer <= 0.0f) {
						enemyMotion.velocity.x = 50.f;
						float ran = (float) (rand()%4-1);
                    	float randomAngle = ran*M_PI/2;
                    	enemyMotion.angle = randomAngle;
                    	enemy.randomMoveTimer = 3.f + rand() % 3;
						enemy.haltTimer = 0.3f;
                	} else {
						enemyMotion.velocity.x = 0.f;
                    	enemy.haltTimer -= step_seconds;
                	}
                } else {
                    enemy.randomMoveTimer -= step_seconds;
                }

				// Check if the player is within the enemy's field of view
            	if (distanceToPlayer<=ENEMY_VERSION_LENGTH&&enemyMotion.angle+ENEMY_VERSION_WIDTH>angleToPlayer&&enemyMotion.angle-ENEMY_VERSION_WIDTH<angleToPlayer) {
                	// Player is within the field of view, take appropriate action
					if (enemy.haltTimer <= 0.0f) {
						enemyMotion.velocity.x = 90.f;
						enemy.seePlayer = true;
						enemy.haltTimer = 0.3f;
                	} else {
						enemyMotion.velocity.x = 0.f;
                    	enemy.haltTimer -= step_seconds;
                	}
            	}
			}
		}
	}
}