#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	
    ComponentContainer<EnterCombatTimer> enterCombatTimer;
    ComponentContainer<Combat> combat;
    ComponentContainer<MainWorld> mainWorld;
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Enemy> mainWorldEnemies;
	ComponentContainer<PinBallEnemy> pinballEnemies;
	ComponentContainer<Room> rooms;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<physObj> physObjs;
	ComponentContainer<playerFlipper> playerFlippers;
	ComponentContainer<mousePos> mousePosArray;
	ComponentContainer<SpriteSheet> spriteSheets;
	ComponentContainer<HighLightEnemy> highLightEnemies;
	ComponentContainer<Light> lights;
	ComponentContainer<PositionKeyFrame> positionKeyFrames;
	ComponentContainer<HealthBar> healthBar;
	ComponentContainer<PinballPlayerStatus> pinballPlayerStatus;
	ComponentContainer<DamageToPlayer> damages;
	ComponentContainer<DamageToEnemy> attackPower;
	ComponentContainer<TemporaryProjectile> temporaryProjectiles;
	ComponentContainer<PinBall> pinBalls;
	ComponentContainer<DropBuff> dropBuffs;
	ComponentContainer<Particle> particles;
	
	// World assets
	ComponentContainer<PlayerBullet> playerBullets;
	ComponentContainer<EnemyBullet> enemyBullets;
	ComponentContainer<Ball> balls;
	ComponentContainer<Spikes> spikes;
	ComponentContainer<Door> doors;

	ComponentContainer<Zombie> zombies;
	ComponentContainer<Sniper> snipers;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
        registry_list.push_back(&combat);
		registry_list.push_back(&mainWorld);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&mainWorldEnemies);
		registry_list.push_back(&pinballEnemies);
		registry_list.push_back(&rooms);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&physObjs);
		registry_list.push_back(&playerFlippers);
		registry_list.push_back(&mousePosArray);
		registry_list.push_back(&spriteSheets);
		registry_list.push_back(&highLightEnemies);
		registry_list.push_back(&lights);
		registry_list.push_back(&positionKeyFrames);
		registry_list.push_back(&healthBar);
		registry_list.push_back(&pinballPlayerStatus);
		registry_list.push_back(&damages);
		registry_list.push_back(&attackPower);
		registry_list.push_back(&temporaryProjectiles);
		registry_list.push_back(&pinBalls);
		registry_list.push_back(&dropBuffs);
		registry_list.push_back(&particles);

		// World assets
		registry_list.push_back(&playerBullets);
		registry_list.push_back(&enemyBullets);
		registry_list.push_back(&balls);
		registry_list.push_back(&spikes);
		registry_list.push_back(&doors);

		registry_list.push_back(&zombies);
		registry_list.push_back(&snipers);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;