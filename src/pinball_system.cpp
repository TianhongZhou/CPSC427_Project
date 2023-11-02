// Header
#include "pinball_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"

// Create the fish world
PinballSystem::PinballSystem()
{

}

PinballSystem::~PinballSystem()
{
//    // Destroy music components
//    if (background_music != nullptr)
//        Mix_FreeMusic(background_music);
//    if (salmon_dead_sound != nullptr)
//        // Mix_FreeChunk(salmon_dead_sound);
//        if (player_attack_sound != nullptr)
//            Mix_FreeChunk(player_attack_sound);
//    Mix_CloseAudio();
//
//    // Destroy all created components
//    registry.clear_all_components();
//
//    // Close the window
//    glfwDestroyWindow(window);
}

// Debugging
//namespace
//{
//    void glfw_err_cb(int error, const char *desc)
//    {
//        fprintf(stderr, "%d: %s", error, desc);
//    }
//}

void PinballSystem::init(GLFWwindow* window_arg, RenderSystem* renderer_arg)
{
    this->window = window_arg;
    this->renderer = renderer_arg;
    // Set all states to default
    init_combat();
//    restart_game();
}

// Update our game world
bool PinballSystem::step(float elapsed_ms_since_last_update)
{
    // Entity blood = registry.healthBar.entities[0];
    // Motion& motion = registry.motions.get(blood);
    // if (motion.scale.x <= 0.5f)
    // {
    // 	GameSceneState = 0;
    // }
    if (registry.pinballEnemies.entities.size()==0) {
        // exit_combat();
        // GameSceneState = 0;
    } else {
        for (Entity entity: registry.pinballEnemies.entities) {
            PinBallEnemy& enemy = registry.pinballEnemies.get(entity);
            if (enemy.currentHealth<=0) {
                for (int j=0; j<enemy.healthBar.size(); j++) {
                    registry.remove_all_components_of(enemy.healthBar[j]);
                }
                registry.remove_all_components_of(entity);
            }
        }
    }

    return true;
}

// Reset the world state to its initial state
void PinballSystem::restart_game()
{
    // Debugging for memory/component leaks
    registry.list_all_components();
    printf("Restarting\n");


    // Remove all entities that we created
    // All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
    while (registry.motions.entities.size() > 0)
        registry.remove_all_components_of(registry.motions.entities.back());

    // Debugging for memory/component leaks
    registry.list_all_components();


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

void PinballSystem::init_combat()
{
    // int w, h;
    // glfwGetWindowSize(window, &w, &h);
    vec2 boundary = { 260+50, 800-50 };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

//    for (int i=0; i<initCombat; i++) {
        Entity pinballenemy = createPinBallEnemy(renderer, {distribution1(gen),180*(2)}, boundary);
        registry.colors.insert(pinballenemy, { distribution2(gen), distribution2(gen), distribution2(gen) });
//    }

    Entity player_ball = createBall(renderer, {400, 400});
    createNewRectangleTiedToEntity(player_ball, 30.f, 30.f, registry.motions.get(player_ball).position, true, 0.7);

    //wall
    Entity leftwall = createPolygonByVertex(renderer, {{220, 749}, {220, 1}, {240, 1}, {240, 749}}, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(leftwall, 20.f, 748.f, registry.motions.get(leftwall).position, false, 1.0);

    Entity rightwall = createPolygonByVertex(renderer, { {820, 749}, {820, 1}, {840, 1}, {840, 749} }, GEOMETRY_BUFFER_ID::OCT);
    createNewRectangleTiedToEntity(rightwall, 20.f, 748.f, registry.motions.get(rightwall).position, false, 1.0);

    //Entity flipper = createPolygonByVertex(renderer, {{300, 600}, {300, 580}, {400, 580}, {400, 600}}, GEOMETRY_BUFFER_ID::RECT);

    //ball
    // Entity squareball = createPolygonByVertex(renderer, { {500, 570}, {500, 520}, {550, 520}, {550, 570} }, GEOMETRY_BUFFER_ID::OCT);
    // createNewRectangleTiedToEntity(squareball, 50.f, 50.f, registry.motions.get(squareball).position, true, 1.0);


    //slide
    Entity leftslide = createPolygonByVertex(renderer, { {220, 750}, {220, 730}, {400, 750}, {400, 730} }, GEOMETRY_BUFFER_ID::RECT);
    createNewRectangleTiedToEntity(leftslide, 180.f, 20.f, registry.motions.get(leftslide).position, false, 1.0);

    Entity rightslide = createPolygonByVertex(renderer, { {660, 750}, {660, 730}, {840, 750}, {840, 730} }, GEOMETRY_BUFFER_ID::RECT);
    createNewRectangleTiedToEntity(rightslide, 180.f, 20.f, registry.motions.get(rightslide).position, false, 1.0);


    //flipper
    Entity flipper = createPolygonByVertex(renderer, {{480, 600}, {480, 580}, {580, 580}, {580, 600}}, GEOMETRY_BUFFER_ID::RECT);
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
void PinballSystem::handle_collisions()
{
    // Loop over all collisions detected by the physics system
    auto &collisionsRegistry = registry.collisions;
    for (uint i = 0; i < collisionsRegistry.components.size(); i++)
    {
        // The entity and its collider
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = collisionsRegistry.components[i].other_entity;

        // For now, we are only interested in collisions that involve the salmon
        if (registry.players.has(entity))
        {
            // Player& player = registry.players.get(entity);

            // Checking Player - Room collisions
            if (registry.rooms.has(entity_other))
            {
                // initiate death unless already dying
                if (!registry.deathTimers.has(entity))
                {
                    // Scream, reset timer, and make the salmon sink
                    registry.deathTimers.emplace(entity);
//                    Mix_PlayChannel(-1, salmon_dead_sound, 0);

                }
            }
                // Checking Player - mainWorldEnemies collisions
            else if (registry.mainWorldEnemies.has(entity_other))
            {
                if (!registry.deathTimers.has(entity))
                {
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

//// Should the game be over ?
//bool PinballSystem::is_over() const
//{
//    return bool(glfwWindowShouldClose(window));
//}

//// On key callback
//void PinballSystem::on_key(int key, int, int action, int mod)
//{
//
//    if (action == GLFW_PRESS)
//    {
//        pressedKeys.insert(key);
//    }
//
//    if (action == GLFW_RELEASE)
//    {
//        pressedKeys.erase(key);
//    }
//
//    Motion& motion = registry.motions.get(player);
//    bool inCombat = GameSceneState || registry.enterCombatTimer.has(player);;
//
//    bool conflictUpAndDown = pressedKeys.count(GLFW_KEY_UP) && pressedKeys.count(GLFW_KEY_DOWN);
//    bool conflictLeftAndRight = pressedKeys.count(GLFW_KEY_LEFT) && pressedKeys.count(GLFW_KEY_RIGHT);
//
//    if (!conflictUpAndDown && (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
//    {
//        motion.velocity.y = (key == GLFW_KEY_UP) ? -200.f : 200.f;
//    }
//    else if (conflictUpAndDown)
//    {
//        motion.velocity.y = 0.f;
//    }
//
//    if (!conflictLeftAndRight && (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS || action == GLFW_REPEAT) && GameSceneState == 0)
//    {
//        motion.velocity.x = (key == GLFW_KEY_LEFT) ? -200.f : 200.f;
//    }
//    else if (conflictLeftAndRight)
//    {
//        motion.velocity.x = 0.f;
//    }
//
//    if (action == GLFW_RELEASE)
//    {
//        if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && !conflictUpAndDown)
//        {
//            if (pressedKeys.count(GLFW_KEY_UP) && GameSceneState == 0)
//            {
//                motion.velocity.y = -200.f;
//            }
//            else if (pressedKeys.count(GLFW_KEY_DOWN) && GameSceneState == 0)
//            {
//                motion.velocity.y = 200.f;
//            }
//            else
//            {
//                motion.velocity.y = 0.f;
//            }
//        }
//
//        if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && !conflictLeftAndRight)
//        {
//            if (pressedKeys.count(GLFW_KEY_LEFT) && GameSceneState == 0)
//            {
//                motion.velocity.x = -200.f;
//            }
//            else if (pressedKeys.count(GLFW_KEY_RIGHT) && GameSceneState == 0)
//            {
//                motion.velocity.x = 200.f;
//            }
//            else
//            {
//                motion.velocity.x = 0.f;
//            }
//        }
//    }
//
//    if ((motion.velocity.x == 0.f) && (motion.velocity.y == 0.f))
//    {
//        if (registry.spriteSheets.has(player))
//        {
//            SpriteSheet& spriteSheet = registry.spriteSheets.get(player);
//            RenderRequest& renderRequest = registry.renderRequests.get(player);
//            renderRequest.used_texture = spriteSheet.origin;
//            registry.spriteSheets.remove(player);
//        }
//    }
//    else
//    {
//        if (!registry.spriteSheets.has(player))
//        {
//            SpriteSheet& spriteSheet = registry.spriteSheets.emplace_with_duplicates(player);
//            spriteSheet.next_sprite = TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET;
//            spriteSheet.frameIncrement = 0.06f;
//            spriteSheet.frameAccumulator = 0.0f;
//            spriteSheet.spriteSheetHeight = 1;
//            spriteSheet.spriteSheetWidth = 6;
//            spriteSheet.totalFrames = 6;
//            spriteSheet.origin = TEXTURE_ASSET_ID::PLAYER;
//            spriteSheet.loop = true;
//            if (motion.velocity.x < 0.f)
//            {
//                spriteSheet.xFlip = true;
//            }
//            RenderRequest& renderRequest = registry.renderRequests.get(player);
//            renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET;
//        }
//    }
//
//    // Resetting game
//    if (action == GLFW_RELEASE && key == GLFW_KEY_R)
//    {
//        int w, h;
//        glfwGetWindowSize(window, &w, &h);
//
//        restart_game();
//    }
//
//    // Exit Combat
//    if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_X)
//    {
//        exit_combat();
//    }
//
//    // Enter Combat
//    if (GameSceneState == 0 && action == GLFW_RELEASE && key == GLFW_KEY_C)
//    {
//        GameSceneState = 1;
//        InitCombat = 1;
//    }
//
//    if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_P)
//    {
//        Entity& flipper = registry.playerFlippers.entities[0];
//
//        physObj& flipperPhys = registry.physObjs.get(flipper);
//
//        flipperPhys.Vertices[3].accel += vec2(0.f, -0.8f);
//    }
//
//    if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
//    {
//        Entity& flipper = registry.playerFlippers.entities[0];
//
//        physObj& flipperPhys = registry.physObjs.get(flipper);
//
//        flipperPhys.Vertices[1].accel += vec2(0.1f, 0.f);
//    }
//
//    if (GameSceneState == 1 && action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
//    {
//        Entity& flipper = registry.playerFlippers.entities[0];
//
//        physObj& flipperPhys = registry.physObjs.get(flipper);
//
//        flipperPhys.Vertices[1].accel += vec2(-0.1f, 0.f);
//    }
//
//    // Debugging
//    if (key == GLFW_KEY_D)
//    {
//        if (action == GLFW_RELEASE)
//            debugging.in_debug_mode = false;
//        else
//            debugging.in_debug_mode = true;
//    }
//
//    // Control the current speed with `<` `>`
//    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
//    {
//        current_speed -= 0.1f;
//        printf("Current speed = %f\n", current_speed);
//    }
//    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
//    {
//        current_speed += 0.1f;
//        printf("Current speed = %f\n", current_speed);
//    }
//    current_speed = fmax(0.f, current_speed);
//
//    // player attack
//    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && GameSceneState == 0) {
//        on_mouse_click(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
//    }
//}
//
//void WorldSystem::on_mouse_move(vec2 mouse_position) {
//
//    if (registry.mousePosArray.size() == 0) {
//        Entity e;
//        mousePos mp;
//        mp.pos = mouse_position;
//        registry.mousePosArray.insert(e, mp);
//    }
//
//    registry.mousePosArray.components[0].pos = mouse_position;
//
//    (vec2) mouse_position; // dummy to avoid compiler warning
//}
//
//void WorldSystem::on_mouse_click(int button, int action, int mods)
//{
//
//    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT && GameSceneState == 0)
//    {
//        Mix_PlayChannel(-1, player_attack_sound, 0);
//
//        bool temp = false;
//        if (registry.spriteSheets.has(player))
//        {
//            SpriteSheet &spriteSheet = registry.spriteSheets.get(player);
//            if (spriteSheet.next_sprite == TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET)
//            {
//                return;
//            }
//            temp = spriteSheet.xFlip;
//        }
//        SpriteSheet &spriteSheet = registry.spriteSheets.emplace_with_duplicates(player);
//        spriteSheet.next_sprite = TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET;
//        spriteSheet.frameIncrement = 0.08f;
//        spriteSheet.frameAccumulator = 0.0f;
//        spriteSheet.spriteSheetHeight = 1;
//        spriteSheet.spriteSheetWidth = 6;
//        spriteSheet.totalFrames = 6;
//        spriteSheet.origin = TEXTURE_ASSET_ID::PLAYER;
//        spriteSheet.xFlip = temp;
//        RenderRequest &renderRequest = registry.renderRequests.get(player);
//        renderRequest.used_texture = TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET;
//
//
//        // Create player bullet
//        Entity entity = createPlayerBullet({ 0,0 }, { 0,0 }); //intialized below
//
//        Motion& motion = registry.motions.get(entity);
//        motion.position = registry.motions.get(player).position;
//
//        float radius = 20; //* (uniform_dist(rng) + 0.3f);
//        motion.scale = { radius, radius };
//
//        vec2 player_v = registry.motions.get(player).velocity;
//        float angle = atan(player_v.y / player_v.x);
//
//        // Set bullet angle and save last angle
//        if (player_v.x == 0.f && player_v.y == 0.f) {
//            angle = last_angle;
//        }
//        else {
//            // CHECK: This is to fix flipping of the axis
//            if (player_v.x < 0) {
//                angle += atan(1) * 4;
//            }
//            last_angle = angle;
//        }
//        motion.angle = angle;
//
//
//        //motion.velocity = vec2(200.f + uniform_dist(rng)*200, 100.f - uniform_dist(rng)*200);
//        //float angle = registry.motions.get(player).angle;
//        motion.velocity = vec2(500.f, 0.f);
//        //motion.velocity.x = velocity.x * cos(angle) + velocity.y * sin(angle);
//        //motion.velocity.y = velocity.x * sin(angle) + velocity.y * cos(angle);
//
//        registry.colors.insert(entity, { 1, 1, 1 });
//    }
//}

void PinballSystem::exit_combat() {
    while (registry.combat.entities.size() > 0)
        registry.remove_all_components_of(registry.combat.entities.back());
}