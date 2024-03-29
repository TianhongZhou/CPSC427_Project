// Header
#include "pinball_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"
#include "world_system.hpp"
#include "swarm_system.hpp"

#include "imgui.h"

//const size_t SWARM_SIZE = 50;
//const int S_CENTER_X = 525;
//const int S_CENTER_Y = 300;
//const int S_RADIUS = 200;
//const int S_SPEED = 1;


float MAX_X_COORD2 = 840.0f;

float MIN_X_COORD2 = 220.0f;

float PLAYER_BASE_ATTACK = 20.0f;

RenderSystem* r;


float DASH_STRENTH = 0.2f;

PinballSystem::PinballSystem() {
   auto curr_level = Entity();
   CombatLevel combatLevel = {1};
   registry.combatLevel.emplace(curr_level, combatLevel);
   this->swarmSystem = SwarmSystem(renderer);

}

PinballSystem::~PinballSystem() {
}

void PinballSystem::init(GLFWwindow *window_arg, RenderSystem *renderer_arg, WorldSystem* world_arg) {
    this->window = window_arg;
    this->renderer = renderer_arg;
    this->world = world_arg;
    this->swarmSystem = SwarmSystem(renderer_arg);
    restart();
    redirect_inputs_pinball();
}

void PinballSystem::redirect_inputs_pinball() {
    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(window, this);
    auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2,
                           int _3) { ((PinballSystem *) glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
    auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1) {
        ((PinballSystem *) glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
    };
    auto mouse_button_redirect = [](GLFWwindow *wnd, int button, int action, int mods) {
        ((PinballSystem *) glfwGetWindowUserPointer(wnd))->on_mouse_click(button, action, mods);
    };
    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);
}

// had to copy accelerateObj here since I don't know if creating a instance of physics system to call the function would cause performance issues

void accelerate2(vec2 acc, Vertex_Phys& obj)
{
    obj.accel += acc;
}


void accelerateObj2(vec2 acc, physObj& obj)
{
    for (int i = 0; i < obj.VertexCount; i++) {
        accelerate2(acc, obj.Vertices[i]);
    }
}

void pinballDash() {
    if (registry.pinballPlayerStatus.components[0].dashCooldown == 0) {

        registry.pinballPlayerStatus.components[0].dashCooldown += 1000.0f;
        physObj& pinballPhys = registry.physObjs.get(registry.pinballPlayerStatus.entities[0]);
        for (int i = 0; i < pinballPhys.VertexCount; i++) {
            pinballPhys.Vertices[i].oldPos = pinballPhys.Vertices[i].pos;

        }
        
        // this would not behave correctly before adding the main enemy
        //if (registry.pinballEnemies.components.size() <= 1) {
        //    printf("no target ");
        //    accelerateObj2(vec2(0.0f, -DASH_STRENTH), pinballPhys);
        //    return;
        //}

        float minDist = 1000.0f;
        vec2 direction = vec2(0.0f, 1.0f);

        for (int i = 0; i < registry.pinballEnemies.components.size(); i++) {
            Entity enemy = registry.pinballEnemies.entities[i];
            float dist = distance(registry.physObjs.get(enemy).center, pinballPhys.center);

            if (dist < minDist) {
                minDist = dist;
                direction = normalize(vec2(registry.physObjs.get(enemy).center.x - pinballPhys.center.x,
                    registry.physObjs.get(enemy).center.y - pinballPhys.center.y));
            }
        }

        accelerateObj2(direction * DASH_STRENTH, pinballPhys);
    }
}


void tractorStep() {
    if (registry.pinballPlayerStatus.components[0].tractorTimer != 0) {


        physObj& pinballPhys = registry.physObjs.get(registry.pinballPlayerStatus.entities[0]);

        physObj flipper = registry.physObjs.get(registry.playerFlippers.entities[0]);


        vec2 direction = vec2(0.0f, 1.0f);


        direction = normalize(vec2(flipper.center.x - pinballPhys.center.x,
            0.0f));

        accelerateObj2(direction * 0.01f, pinballPhys);

        for (int i = 0; i < registry.temporaryProjectiles.size(); i++){
            physObj& ball = registry.physObjs.get(registry.temporaryProjectiles.entities[i]);
            direction = normalize(vec2(flipper.center.x - ball.center.x,
                0.0f));

            accelerateObj2(direction * 0.01f, ball);
        }



    }
}

void countdown(float& timer, float ms) {
    if (timer != 0.0f) {
        if (timer > ms) {
            timer -= ms;
        }
        else {
            timer = 0.0f;
        }
    }
}


void updateTimers(float ms) {
    if (registry.pinballPlayerStatus.components.size() != 0) {

        //float& invTimer = registry.pinballPlayerStatus.components[0].invincibilityTimer;

//if (invTimer != 0.0f) {

//    if (invTimer > ms) {
//        invTimer -= ms;
//    }
//    else {
//        invTimer = 0.0f;
//    }
//}

        countdown(registry.pinballPlayerStatus.components[0].invincibilityTimer, ms);
        countdown(registry.pinballPlayerStatus.components[0].antiGravityTimer, ms);
        countdown(registry.pinballPlayerStatus.components[0].highGravityTimer, ms);
        countdown(registry.pinballPlayerStatus.components[0].dashCooldown, ms);
        countdown(registry.pinballPlayerStatus.components[0].focusTimer, ms);
        countdown(registry.pinballPlayerStatus.components[0].tractorTimer, ms);

        for (int i = 0; i < registry.pinballEnemies.components.size(); i++) {
            countdown(registry.pinballEnemies.components[i].invincibilityTimer, ms);
            countdown(registry.pinballEnemies.components[i].attackTimer, ms);
            // making the color the attack cd indicator
            registry.colors.get(registry.pinballEnemies.entities[i]).r = clamp((registry.pinballEnemies.components[i].attackTimer * 0.001f)/5.0f, 0.0f, 1.0f);
        }

        for (int i = 0; i < registry.temporaryProjectiles.components.size(); i++) {
            countdown(registry.temporaryProjectiles.components[i].timeLeft, ms);
            if (registry.temporaryProjectiles.components[i].timeLeft == 0 && !registry.temporaryProjectiles.components[i].bonusBall) {
                registry.remove_all_components_of(registry.temporaryProjectiles.entities[i]);
            }
        }

    }

}


void PinballSystem::stepEnemyAttack() {
    for (int i = 0; i < registry.pinballEnemies.components.size(); i++) {
        if (registry.pinballEnemies.components[i].attackTimer == 0.0f) {
            if (registry.pinballEnemies.components[i].attackType == 0) {




                printf(" attackType0 ");

                PinBall& pinball = registry.pinBalls.components[0];

                physObj enemyObj = registry.physObjs.get(registry.pinballEnemies.entities[i]);
                vec2 spawnPos = vec2(enemyObj.center.x, enemyObj.center.y + 50.0f);

                Entity projectile_ball = createBall(r, spawnPos, pinball.pinBallSize, 0.f);
                createNewRectangleTiedToEntity(projectile_ball, pinball.pinBallSize, pinball.pinBallSize, registry.motions.get(projectile_ball).position, true, 1);
                registry.colors.insert(projectile_ball, { 1.f, 0.f, 0.f});

                TemporaryProjectile temp;
                temp.hitsLeft = 1;
                temp.bonusBall = false;
                temp.timeLeft = 4000.0f;
                DamageToPlayer d;
                d.damage = 20.0f;
                DamageToEnemy d2;
                d2.damage = 20.0f;

                registry.attackPower.emplace(projectile_ball, d2);
                registry.damages.emplace(projectile_ball, d);
                registry.temporaryProjectiles.emplace(projectile_ball, temp);


            }
            else {
                registry.pinballPlayerStatus.components[0].highGravityTimer += 500.0f;
                printf(" attackType1 ");
            }

            registry.pinballEnemies.components[i].attackTimer += registry.pinballEnemies.components[i].attackCooldown;
        }
    }
}


// Update our game world
bool PinballSystem::step(float elapsed_ms_since_last_update) {
    // Entity blood = registry.healthBar.entities[0];
    // Motion& motion = registry.motions.get(blood);
    // if (motion.scale.x <= 0.5f)
    // {
    // 	GameSceneState = 0;
    // }
    float step_seconds = elapsed_ms_since_last_update / 1000.f;

    if (backgrounds!=NULL) {
        float relativePosX = (window_width_px/2 -
        (registry.physObjs.get(registry.playerFlippers.entities[0]).center.x))/window_width_px;
        registry.paras.get(backgrounds[2]).offset = -(relativePosX/10);
        registry.paras.get(backgrounds[3]).offset = -(relativePosX/7);
        registry.paras.get(backgrounds[4]).offset = -(relativePosX/5);
        registry.paras.get(backgrounds[5]).offset = -(relativePosX/3);
        registry.paras.get(backgrounds[6]).offset = -(relativePosX/2);
        registry.paras.get(backgrounds[7]).offset = -(relativePosX/2);
    }

    if (registry.pinballEnemies.entities.size() == 0 && registry.swarmKing.entities.empty()) {
        // exit_combat();
        // GameSceneState = 0;
    } else {
        if (particleSpawnTimer > 0.01f) {
            float pinballRadius = registry.pinBalls.components[0].pinBallSize;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> distRadius(0, pinballRadius);
            std::uniform_real_distribution<> distAngle(0.f, 2.f * M_PI);
            std::normal_distribution<> distLifeSpan(1.0f, 0.2f);
            std::normal_distribution<> distVel(0.f, 1.f);
            int numParticles = registry.pinBalls.components[0].pinBallSize;
            for (Entity entity: registry.balls.entities) {
                if (registry.balls.get(entity).trail>0.f) {
                physObj ball = registry.physObjs.get(entity);
                vec2 center = {0.f, 0.f};
                for (int i=0; i<ball.VertexCount; i++) {
                    center+=ball.Vertices[i].pos;
                }
                center=center/(float) ball.VertexCount;
                for (int i = 0; i < numParticles; ++i) {
                    float randomRadius = distRadius(gen);
                    float randomAngle = distAngle(gen);
                    vec3 color = {0.f,0.f,1.f};
                    vec3 grey = { 0.5f, 0.5f, 0.5f };
                    color = (color + grey) * 0.5f;
                    color *= ((pinballRadius-randomRadius)/pinballRadius+1)/2;
                    vec2 pos = {center.x + randomRadius * std::cos(randomAngle), center.y + randomRadius * std::sin(randomAngle)};
                    createParticle(r, pos, 1.5f, {distVel(gen),distVel(gen)}, color, distLifeSpan(gen));
                }
                // for (int i=0; i<ball.EdgesCount; i++) {
                //     vec2 pos1 = ball.Vertices[ball.Edges[i].v1].pos;
                //     vec2 pos2 = ball.Vertices[ball.Edges[i].v2].pos;
                //     vec2 diff = pos1-pos2;
                //     for (int j=0; j<ball.Edges[i].len; j+=5) {
                //         vec2 pos = pos2+(diff/ball.Edges[i].len* (float)j);
                //         createParticle(r, pos, 1.f, {0.f,0.f}, {1.f,0.f,0.f}, 2.f);
                //     }
                // }
                }
            }
            particleSpawnTimer=0.f;
        } else {
            particleSpawnTimer+=step_seconds;
        }

        for (Entity entity: registry.pinballEnemies.entities) {
            PinBallEnemy &enemy = registry.pinballEnemies.get(entity);
            if (enemy.currentHealth <= 0) {
                for (int j = 0; j < enemy.healthBar.size(); j++) {
                    registry.remove_all_components_of(enemy.healthBar[j]);
                }

                if (registry.swarmKing.has(entity)) {
                    for (Entity se: registry.swarmEnemies.entities) {
                        registry.remove_all_components_of(se);
                    }
//                    delete swarmSystem;
                }

                Mix_PlayChannel(-1, registry.sfx.components[0].enemy_death_sound, 0);

                registry.remove_all_components_of(entity);
            }
        }

    }
    if (registry.swarmKing.size() > 0) {
        swarmSystem.handle_swarm_collision();
        swarmSystem.update_swarm_motion();
//        update_swarm_motion();

    }

    if (registry.pinballEnemies.entities.size() <= 0) {
        if (registry.playerFlippers.components[0].exit_timer>=4.f) {
            registry.playerFlippers.components[0].exit_timer=0.f;
            exit_combat();
        } else {
            registry.playerFlippers.components[0].exit_timer+=step_seconds;
        }
        updateTimers(elapsed_ms_since_last_update);
        return true;
    }

    updateTimers(elapsed_ms_since_last_update);
    //stepEnemyAttack();
    if (registry.roomLevel.components[0].counter > 3) {
        stepEnemyAttack();
    }
    tractorStep();


    if (registry.pinballPlayerStatus.components[0].health <= 0.0f) {
        printf("\n-You Died-\n");
        exit_combat();
        //world->restart_game();
        registry.players.components[0].currentHealth -= 40.f; 
    }

    return true;
}

//void spawnBonusBall() {
//    if (registry.pinballPlayerStatus.components[0].comboCounter >= 5) {
//
//        registry.pinballPlayerStatus.components[0].comboCounter - 5;
//
//        printf("Spawned bonus ball! Combo count=%i ", registry.pinballPlayerStatus.components[0].comboCounter);
//
//        vec2 spawnPos = vec2(registry.physObjs.get(registry.playerFlippers.entities[0]).center.x,
//            registry.physObjs.get(registry.playerFlippers.entities[0]).center.y - 50.0f);
//
//        Entity projectile_ball = createBall(renderer, spawnPos);
//        createNewRectangleTiedToEntity(projectile_ball, 30.f, 30.f, registry.motions.get(projectile_ball).position, true, 1);
//
//        TemporaryProjectile temp;
//        temp.hitsLeft = 1;
//        temp.bonusBall = true;
//        DamageToPlayer d;
//        d.damage = 5.0f;
//        DamageToEnemy d2;
//        d2.damage = PLAYER_BASE_ATTACK * 0.5;
//
//        registry.attackPower.emplace(projectile_ball, d2);
//        registry.damages.emplace(projectile_ball, d);
//        registry.temporaryProjectiles.emplace(projectile_ball, temp);
//    }
//    else {
//        printf("Not enough combo ");
//    }
//}


// On key callback
void PinballSystem::on_key(int key, int, int action, int mod) {

    // Debugging
    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_RELEASE)
            debugging.in_debug_mode = false;
        else
            debugging.in_debug_mode = true;
    }

    // enemy kill switch
    // TODO: comment
    if (key == GLFW_KEY_K && action == GLFW_RELEASE)
    {
        for (Entity entity: registry.pinballEnemies.entities) {
            PinBallEnemy &enemy = registry.pinballEnemies.get(entity);
            if (enemy.currentHealth >= 0) {
                enemy.currentHealth = 1;
            }
        }
    }

    //// Resetting game
    //if (action == GLFW_RELEASE && key == GLFW_KEY_R)
    //{
    //    exit_combat();
    //    world->restart_game();
    //}

    if (action == GLFW_RELEASE && key == GLFW_KEY_X)
    {
        exit_combat();
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
    {

        Mix_PlayChannel(-1, world->flipper_sound, 0);

        Entity& flipper = registry.playerFlippers.entities[0];

        physObj& flipperPhys = registry.physObjs.get(flipper);

        flipperPhys.Vertices[0].accel += vec2(0.f, -0.8f);
        flipperPhys.Vertices[1].accel += vec2(0.f, -0.8f);
        
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_U)
    {
        if (registry.pinBalls.components[0].antiGravityCount > 0) {
            registry.pinBalls.components[0].antiGravityCount--;
            registry.pinballPlayerStatus.components[0].antiGravityTimer += 5000.0f;
        }
        else {
            printf(" No antiGrav charge ");
        }
        
    }

    //if (action == GLFW_RELEASE && key == GLFW_KEY_I)
    //{
    //    registry.pinballPlayerStatus.components[0].highGravityTimer += 5000.0f;
    //}

    if (action == GLFW_RELEASE && key == GLFW_KEY_F)
    {
        if (registry.pinballPlayerStatus.components[0].dashCooldown == 0) {
            Mix_PlayChannel(-1, world->dash_sound, 0); // had to do this check here because of strange reference loss
        }
        pinballDash();
    }

    //if (action == GLFW_RELEASE && key == GLFW_KEY_K)
    //{
    //    PinBall& pinBall = registry.pinBalls.components[0];
    //    Entity projectile_ball = createBall(renderer, { 400, 400 }, pinBall.pinBallSize, 0.f);
    //    createNewRectangleTiedToEntity(projectile_ball, pinBall.pinBallSize, pinBall.pinBallSize, registry.motions.get(projectile_ball).position, true, 1);

    //    TemporaryProjectile temp;
    //    temp.hitsLeft = 2;
    //    temp.bonusBall = false;
    //    DamageToPlayer d;
    //    d.damage = 20.0f;
    //    DamageToEnemy d2;
    //    d2.damage = registry.pinBalls.components[0].pinBallDamage * 2.0f;

    //    registry.attackPower.emplace(projectile_ball, d2);
    //    registry.damages.emplace(projectile_ball, d);
    //    registry.temporaryProjectiles.emplace(projectile_ball, temp);

    //}

    if (action == GLFW_RELEASE && key == GLFW_KEY_Q)
    {
        if (registry.pinballPlayerStatus.components[0].comboCounter >= 5) {

            registry.pinballPlayerStatus.components[0].comboCounter -= 5;
            printf("Spawned bonus ball! Combo count=%i ", registry.pinballPlayerStatus.components[0].comboCounter);

            vec2 spawnPos = vec2(registry.physObjs.get(registry.playerFlippers.entities[0]).center.x,
                registry.physObjs.get(registry.playerFlippers.entities[0]).center.y - 50.0f);

            PinBall& pinBall = registry.pinBalls.components[0];
            Entity projectile_ball = createBall(renderer, spawnPos, pinBall.pinBallSize, 0.f);
            createNewRectangleTiedToEntity(projectile_ball, pinBall.pinBallSize, pinBall.pinBallSize, registry.motions.get(projectile_ball).position, true, 1);

            TemporaryProjectile temp;
            temp.hitsLeft = 1;
            temp.bonusBall = true;
            DamageToPlayer d;
            d.damage = 5.0f;
            DamageToEnemy d2;
            d2.damage = registry.pinBalls.components[0].pinBallDamage * 0.5;

            registry.attackPower.emplace(projectile_ball, d2);
            registry.damages.emplace(projectile_ball, d);
            registry.temporaryProjectiles.emplace(projectile_ball, temp);
        }
        else {
            printf("Not enough combo ");
        }
    }

    //if (action == GLFW_RELEASE && key == GLFW_KEY_W)
    //{
    //    registry.pinballPlayerStatus.components[0].focusTimer = 600.0f;
    //}

    if (action == GLFW_RELEASE && key == GLFW_KEY_M)
    {
        if (registry.pinBalls.components[0].tractorBeamCount > 0) {
            registry.pinBalls.components[0].tractorBeamCount--;
            registry.pinballPlayerStatus.components[0].tractorTimer = 10000.0f;
        }
        else {
            printf(" No tractorBeam charge ");
        }
    }


    //if (action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
    //{
    //    Entity& flipper = registry.playerFlippers.entities[0];
    //    physObj& flipperPhys = registry.physObjs.get(flipper);
    //    flipperPhys.Vertices[1].accel += vec2(0.2f, 0.f);
    //}

    //if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
    //{
    //    Entity& flipper = registry.playerFlippers.entities[0];
    //    physObj& flipperPhys = registry.physObjs.get(flipper);
    //    flipperPhys.Vertices[1].accel += vec2(-0.2f, 0.f);
    //}
}




void PinballSystem::on_mouse_move(vec2 mouse_position) {
    (vec2) mouse_position;

    float width = 100.0f;
    float height = 20.0f;


    if ((mouse_position.x * (float)window_width_px / (float)MonitorWidth + width / 2) < MAX_X_COORD2  && (mouse_position.x * (float)window_width_px / (float)MonitorWidth - width / 2) > MIN_X_COORD2) {
        physObj& flipper = registry.physObjs.get(registry.playerFlippers.entities[0]);
        float xPos = 0;
        for (int i=0; i<flipper.VertexCount; i++) {
            if (i==0 || i==3) xPos = mouse_position.x * (float)window_width_px / (float)MonitorWidth - width / 2;
            else xPos = mouse_position.x * (float)window_width_px / (float)MonitorWidth + width / 2;
            flipper.Vertices[i].pos.x = xPos;
            flipper.Vertices[i].oldPos.x = xPos;
        }
    }

}

void PinballSystem::on_mouse_click(int button, int action, int mods) {
    (int) button;
    (int) action;
    (int) mods;

    if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (registry.pinballPlayerStatus.components[0].dashCooldown == 0) {
            Mix_PlayChannel(-1, world->dash_sound, 0); // had to do this check here because of strange reference loss
        }
        pinballDash();
    }

    if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        Mix_PlayChannel(-1, world->flipper_sound, 0);

        Entity& flipper = registry.playerFlippers.entities[0];

        physObj& flipperPhys = registry.physObjs.get(flipper);

        flipperPhys.Vertices[0].accel += vec2(0.f, -0.8f);
        flipperPhys.Vertices[1].accel += vec2(0.f, -0.8f);
    }

}


void PinballSystem::restart() {
    Entity c = registry.combatLevel.entities[0];
    int& curr_level = registry.combatLevel.get(c).counter;
    int room_level = registry.roomLevel.components[0].counter;
    curr_level = room_level;

    switch (curr_level) {
        case -1:
            start_test_level();
        case 1:
            start_level_1();
            break;
        case 2:
            start_level_2();
            break;
        case 3:
            start_level_3();
            break;
        case 5:
            start_level_1();
            break;
        case 6:
            start_level_2();
            break;
        case 7:
            start_level_3();
            break;
        default:
            start_level_1();
    }

//    curr_level += 1;
}

// initializes the room, ball, and flipper
void PinballSystem::start_base_level() {

    backgrounds = createPinballRoom(renderer, { 600, 400 }, window);
    r = renderer;
    vec2 boundary = {260 + 70, 800 - 70};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

    //flipper
    Entity flipper = createPinballFlipper(renderer, { {480, 600},
                                                      {480, 580},
                                                      {580, 580},
                                                      {580, 600} }, GEOMETRY_BUFFER_ID::RECT);
    createNewRectangleTiedToEntity(flipper, 100.f, 20.f, registry.motions.get(flipper).position, true, 0.0);

    playerFlipper pf;
    registry.playerFlippers.insert(flipper, pf);

    PinBall& pinBall = registry.pinBalls.components[0];
    Entity player_ball = createBall(renderer, { 400, 400 }, pinBall.pinBallSize, 1.f, true);
    //createNewRectangleTiedToEntity(player_ball, 50.f * MonitorScreenRatio, 50.f * MonitorScreenRatio * 1.2f, registry.motions.get(player_ball).position, true, 1);
    createNewRectangleTiedToEntity(player_ball, pinBall.pinBallSize, pinBall.pinBallSize, registry.motions.get(player_ball).position, true, 1);

    // setting up player status for pinball
    PinballPlayerStatus status;
    status.health = 100.0f;
    status.invincibilityTimer = 0.0f;
    status.antiGravityTimer = 0.0f;
    status.highGravityTimer = 0.0f;
    status.comboCounter = 0;

    // setting up playerball self damage
    DamageToPlayer playerballDamage;
    playerballDamage.damage = 20.0f;

    DamageToEnemy playerballAttack;
    playerballAttack.damage = registry.pinBalls.components[0].pinBallDamage;

    registry.pinballPlayerStatus.emplace(player_ball, status);
    registry.damages.emplace(player_ball, playerballDamage);
    registry.attackPower.emplace(player_ball, playerballAttack);

    //wall
    Entity leftwall = createPinballWall(renderer, {{220, 749},
                                                   {220, 1},
                                                   {240, 1},
                                                   {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(leftwall, 20.f, 748.f, registry.motions.get(leftwall).position, false, 1.0);

    Entity rightwall = createPinballWall(renderer, {{820, 749},
                                                    {820, 1},
                                                    {840, 1},
                                                    {840, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(rightwall, 20.f, 748.f, registry.motions.get(rightwall).position, false, 1.0);
}

// contains the original initialization from M3
void PinballSystem::start_test_level() {
    // int w, h;
    // glfwGetWindowSize(window, &w, &h);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    backgrounds = createPinballRoom(renderer, { 600, 400 }, window);
    r = renderer;
    vec2 boundary = {260 + 70, 800 - 70};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

//    spawn_swarm(boundary);

    //flipper
    Entity flipper = createPinballFlipper(renderer, { {480, 600},
                                                      {480, 580},
                                                      {580, 580},
                                                      {580, 600} }, GEOMETRY_BUFFER_ID::RECT);
    createNewRectangleTiedToEntity(flipper, 100.f, 20.f, registry.motions.get(flipper).position, true, 0.0);


    //enemy
    // Entity enemyobj = createPolygonByVertex(renderer, { {360, 380}, {360, 320}, {520, 320}, {520, 380} }, GEOMETRY_BUFFER_ID::OCT);
    // createNewRectangleTiedToEntity(enemyobj, 120.f, 50.f, registry.motions.get(enemyobj).position, false, 1.0);
    // PinBallEnemy &pinballEnemy = registry.pinballEnemies.emplace(enemyobj);
    // registry.colors.insert(enemyobj, { 0.6, 0, 0 });

    playerFlipper pf;
    registry.playerFlippers.insert(flipper, pf);

    PinBall& pinBall = registry.pinBalls.components[0];
    Entity player_ball = createBall(renderer, { 400, 400 }, pinBall.pinBallSize, 1.f);
    //createNewRectangleTiedToEntity(player_ball, 50.f * MonitorScreenRatio, 50.f * MonitorScreenRatio * 1.2f, registry.motions.get(player_ball).position, true, 1);
    createNewRectangleTiedToEntity(player_ball, pinBall.pinBallSize, pinBall.pinBallSize, registry.motions.get(player_ball).position, true, 1);


    // setting up player status for pinball
    PinballPlayerStatus status;
    status.health = 100.0f;
    status.invincibilityTimer = 0.0f;
    status.antiGravityTimer = 0.0f;
    status.highGravityTimer = 0.0f;
    status.comboCounter = 0;
    status.focusTimer = 0.0f;
    status.tractorTimer = 0.0f;

    // setting up playerball self damage
    DamageToPlayer playerballDamage;
    playerballDamage.damage = 20.0f;

    DamageToEnemy playerballAttack;
    playerballAttack.damage = registry.pinBalls.components[0].pinBallDamage;

    registry.pinballPlayerStatus.emplace(player_ball, status);
    registry.damages.emplace(player_ball, playerballDamage);
    registry.attackPower.emplace(player_ball, playerballAttack);

    Entity pinballenemyMain = createPinBallEnemy(renderer, vec2(525,180), boundary,2.0f, 0, 3000.0f, 1.0);

    registry.colors.insert(pinballenemyMain, { distribution2(gen), distribution2(gen), distribution2(gen) });


    Entity pinballenemy = createPinBallEnemy(renderer, vec2(525, 90), boundary, 2.0f, 1, 5000.0f, 1.f);

    registry.colors.insert(pinballenemy, { distribution2(gen), distribution2(gen), distribution2(gen) });


    // for (int i=0; i<NUM_ENEMIES; i++) {
    // Entity pinballenemy = createPinBallEnemy(renderer, {distribution1(gen), 180 * (2)}, boundary,1.0f);
    // registry.colors.insert(pinballenemy, {distribution2(gen), distribution2(gen), distribution2(gen)});
    // }


    //wall
    Entity leftwall = createPinballWall(renderer, {{220, 749},
                                                   {220, 1},
                                                   {240, 1},
                                                   {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(leftwall, 20.f, 748.f, registry.motions.get(leftwall).position, false, 1.0);

    Entity rightwall = createPinballWall(renderer, {{820, 749},
                                                    {820, 1},
                                                    {840, 1},
                                                    {840, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(rightwall, 20.f, 748.f, registry.motions.get(rightwall).position, false, 1.0);

    //Entity flipper = createPolygonByVertex(renderer, {{300, 600}, {300, 580}, {400, 580}, {400, 600}}, GEOMETRY_BUFFER_ID::RECT);

    //ball
    // Entity squareball = createPolygonByVertex(renderer, { {500, 570}, {500, 520}, {550, 520}, {550, 570} }, GEOMETRY_BUFFER_ID::OCT);
    // createNewRectangleTiedToEntity(squareball, 50.f, 50.f, registry.motions.get(squareball).position, true, 1.0);


    ////slide
    //Entity leftslide = createPolygonByVertex(renderer, {{220, 750},
    //                                                    {220, 730},
    //                                                    {400, 750},
    //                                                    {400, 730}}, GEOMETRY_BUFFER_ID::RECT);
    //createNewRectangleTiedToEntity(leftslide, 180.f, 20.f, registry.motions.get(leftslide).position, false, 1.0);

    //Entity rightslide = createPolygonByVertex(renderer, {{660, 750},
    //                                                     {660, 730},
    //                                                     {840, 750},
    //                                                     {840, 730}}, GEOMETRY_BUFFER_ID::RECT);
    //createNewRectangleTiedToEntity(rightslide, 180.f, 20.f, registry.motions.get(rightslide).position, false, 1.0);

}

void PinballSystem::start_level_1() {
    start_base_level();

    r = renderer;
    vec2 boundary = {260 + 70, 800 - 70};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

    Entity pinballenemyMain = createPinBallEnemy(renderer, vec2(525,180), boundary,2.0f, 0, 3000.0f, 1.0);
    PinBallEnemy& pinballEnemy = registry.pinballEnemies.get(pinballenemyMain);
    pinballEnemy.maxHealth = 50.f;
    pinballEnemy.currentHealth = 50.f;

    registry.colors.insert(pinballenemyMain, { distribution2(gen), distribution2(gen), distribution2(gen) });
}

void PinballSystem::start_level_2() {
    start_base_level();
    r = renderer;
    vec2 boundary = {260 + 70, 800 - 70};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

    Entity pinballenemyMain = createPinBallEnemy(renderer, vec2(525,180), boundary,2.0f, 0, 3000.0f, 1.0);
    registry.colors.insert(pinballenemyMain, { distribution2(gen), distribution2(gen), distribution2(gen) });

    Entity pinballenemy = createPinBallEnemy(renderer, vec2(525, 90), boundary, 2.0f, 1, 5000.0f, 1.f);
    registry.colors.insert(pinballenemy, { distribution2(gen), distribution2(gen), distribution2(gen) });

    PinBallEnemy& pinballEnemy = registry.pinballEnemies.get(pinballenemyMain);
    pinballEnemy.maxHealth = 150.f;
    pinballEnemy.currentHealth = 150.f;

    PinBallEnemy& pinballEnemy2 = registry.pinballEnemies.get(pinballenemy);
    pinballEnemy2.maxHealth = 150.f;
    pinballEnemy2.currentHealth = 150.f;
}

void PinballSystem::start_level_3() {
    start_base_level();
    vec2 boundary = {260 + 70, 800 - 70};
    this->swarmSystem = SwarmSystem(renderer);
    swarmSystem.spawn_swarm(boundary);
//    spawn_swarm(boundary);
}

// Compute collisions between entities
void PinballSystem::handle_collisions() {
    // Loop over all collisions detected by the physics system
    auto &collisionsRegistry = registry.collisions;
    for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
        // The entity and its collider
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = collisionsRegistry.components[i].other_entity;

        // For now, we are only interested in collisions that involve the salmon
        if (registry.players.has(entity)) {
            // Player& player = registry.players.get(entity);

            // Checking Player - Room collisions
            if (registry.rooms.has(entity_other)) {
                // initiate death unless already dying
                if (!registry.deathTimers.has(entity)) {
                    // Scream, reset timer, and make the salmon sink
                    registry.deathTimers.emplace(entity);
//                    Mix_PlayChannel(-1, salmon_dead_sound, 0);

                }
            }
                // Checking Player - mainWorldEnemies collisions
            else if (registry.mainWorldEnemies.has(entity_other)) {
                if (!registry.deathTimers.has(entity)) {
                    // chew, count points, and set the LightUp timer
                    registry.remove_all_components_of(entity_other);
                    // Mix_PlayChannel(-1, player_attack_sound, 0);
                }
            }
        }
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

void PinballSystem::exit_combat() {
    while (registry.combat.entities.size() > 0)
        registry.remove_all_components_of(registry.combat.entities.back());

    // FIXME: awkward logic. deleting without this check causes crashes on non swarm levels
//    if (registry.combatLevel.components[0].counter == 3) {
//        delete swarmSystem;
//    }
    world->redirect_inputs_world();
    GameSceneState = 0;
    registry.motions.get(registry.players.entities[0]).velocity = vec2(0.f,0.f);
}


