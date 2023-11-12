// Header
#include "pinball_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"
#include "world_system.hpp"

const size_t NUM_ENEMIES = 2;


float DASH_STRENTH = 0.5f;

PinballSystem::PinballSystem() {

}

PinballSystem::~PinballSystem() {
}

void PinballSystem::init(GLFWwindow *window_arg, RenderSystem *renderer_arg, WorldSystem* world_arg) {
    this->window = window_arg;
    this->renderer = renderer_arg;
    this->world = world_arg;
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


        // this would not behave correctly before adding the main enemy
        if (registry.pinballEnemies.components.size() <= 1) {
            printf("no target ");
            accelerateObj2(vec2(0.0f, -DASH_STRENTH), pinballPhys);
            return;
        }





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



void updateTimers(float ms) {

    if (registry.pinballPlayerStatus.components.size() != 0) {

        float& invTimer = registry.pinballPlayerStatus.components[0].invincibilityTimer;

        if (invTimer != 0.0f) {

            if (invTimer > ms) {
                invTimer -= ms;
            }
            else {
                invTimer = 0.0f;
            }
        }


        float& antiTimer = registry.pinballPlayerStatus.components[0].antiGravityTimer;

        if (antiTimer != 0.0f) {

            if (antiTimer > ms) {
                antiTimer -= ms;
            }
            else {
                antiTimer = 0.0f;
            }
        }


        float& highGravTimer = registry.pinballPlayerStatus.components[0].highGravityTimer;

        if (highGravTimer != 0.0f) {

            if (highGravTimer > ms) {
                highGravTimer -= ms;
            }
            else {
                highGravTimer = 0.0f;
            }
        }


        float& dashCd = registry.pinballPlayerStatus.components[0].dashCooldown;

        if (dashCd != 0.0f) {

            if (dashCd > ms) {
                dashCd -= ms;
            }
            else {
                dashCd = 0.0f;
            }
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
    if (registry.pinballEnemies.entities.size() == 0) {
        // exit_combat();
        // GameSceneState = 0;
    } else {
        for (Entity entity: registry.pinballEnemies.entities) {
            PinBallEnemy &enemy = registry.pinballEnemies.get(entity);
            if (enemy.currentHealth <= 0) {
                for (int j = 0; j < enemy.healthBar.size(); j++) {
                    registry.remove_all_components_of(enemy.healthBar[j]);
                }
                registry.remove_all_components_of(entity);
            }
        }
    }

    updateTimers(elapsed_ms_since_last_update);


    return true;
}

// On key callback
void PinballSystem::on_key(int key, int, int action, int mod) {
    if (action == GLFW_RELEASE && key == GLFW_KEY_X)
    {
        exit_combat();
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
    {
        Entity& flipper = registry.playerFlippers.entities[0];

        physObj& flipperPhys = registry.physObjs.get(flipper);

        flipperPhys.Vertices[0].accel += vec2(0.f, -0.8f);
        flipperPhys.Vertices[1].accel += vec2(0.f, -0.8f);
    }


    if (action == GLFW_RELEASE && key == GLFW_KEY_U)
    {
        registry.pinballPlayerStatus.components[0].antiGravityTimer += 5000.0f;
    }


    if (action == GLFW_RELEASE && key == GLFW_KEY_I)
    {
        registry.pinballPlayerStatus.components[0].highGravityTimer += 5000.0f;
    }


    if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT_SHIFT)
    {
        pinballDash();
    }



    if (action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
    {
        Entity& flipper = registry.playerFlippers.entities[0];

        physObj& flipperPhys = registry.physObjs.get(flipper);

        flipperPhys.Vertices[1].accel += vec2(0.2f, 0.f);
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
    {
        Entity& flipper = registry.playerFlippers.entities[0];

        physObj& flipperPhys = registry.physObjs.get(flipper);

        flipperPhys.Vertices[1].accel += vec2(-0.2f, 0.f);
    }
}

void PinballSystem::on_mouse_move(vec2 mouse_position) {
    (vec2) mouse_position;
}

void PinballSystem::on_mouse_click(int button, int action, int mods) {
    (int) button;
    (int) action;
    (int) mods;
}

void PinballSystem::restart() {
    // int w, h;
    // glfwGetWindowSize(window, &w, &h);
    vec2 boundary = {260 + 50, 800 - 50};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);


 

    Entity player_ball = createBall(renderer, { 400, 400 });
    createNewRectangleTiedToEntity(player_ball, 30.f, 30.f, registry.motions.get(player_ball).position, true, 0.7);

    // setting up player status for pinball
    PinballPlayerStatus status;
    status.health = 100.0f;
    status.invincibilityTimer = 0.0f;
    status.antiGravityTimer = 0.0f;
    status.highGravityTimer = 0.0f;

    // setting up playerball self damage
    DamageToPlayer playerballDamage;
    playerballDamage.damage = 20.0f;


    //





    for (int i=0; i<NUM_ENEMIES; i++) {
    Entity pinballenemy = createPinBallEnemy(renderer, {distribution1(gen), 180 * (2)}, boundary);
    registry.colors.insert(pinballenemy, {distribution2(gen), distribution2(gen), distribution2(gen)});
    }



    registry.pinballPlayerStatus.emplace(player_ball, status);
    registry.damages.emplace(player_ball, playerballDamage);

    //wall
    Entity leftwall = createPolygonByVertex(renderer, {{220, 749},
                                                       {220, 1},
                                                       {240, 1},
                                                       {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(leftwall, 20.f, 748.f, registry.motions.get(leftwall).position, false, 1.0);

    Entity rightwall = createPolygonByVertex(renderer, {{820, 749},
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


    //flipper
    Entity flipper = createPolygonByVertex(renderer, {{480, 600},
                                                      {480, 580},
                                                      {580, 580},
                                                      {580, 600}}, GEOMETRY_BUFFER_ID::RECT);
    createNewRectangleTiedToEntity(flipper, 100.f, 20.f, registry.motions.get(flipper).position, true, 0.0);


    //enemy
    // Entity enemyobj = createPolygonByVertex(renderer, { {360, 380}, {360, 320}, {520, 320}, {520, 380} }, GEOMETRY_BUFFER_ID::OCT);
    // createNewRectangleTiedToEntity(enemyobj, 120.f, 50.f, registry.motions.get(enemyobj).position, false, 1.0);
    // PinBallEnemy &pinballEnemy = registry.pinballEnemies.emplace(enemyobj);
    // registry.colors.insert(enemyobj, { 0.6, 0, 0 });

    playerFlipper pf;
    registry.playerFlippers.insert(flipper, pf);
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
    world->redirect_inputs_world();
    GameSceneState = 0;
}


