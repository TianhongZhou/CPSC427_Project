// Header
#include "pinball_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <algorithm>

#include "physics_system.hpp"
#include "world_system.hpp"

const size_t SWARM_SIZE = 50;
const int S_CENTER_X = 525;
const int S_CENTER_Y = 200;
const int S_RADIUS = 200;
const int S_SPEED = 1;


float MAX_X_COORD2 = 840.0f;

float MIN_X_COORD2 = 220.0f;

float PLAYER_BASE_ATTACK = 20.0f;

RenderSystem* r;


float DASH_STRENTH = 0.2f;

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

        for (int i = 0; i < registry.pinballEnemies.components.size(); i++) {
            countdown(registry.pinballEnemies.components[i].invincibilityTimer, ms);
            countdown(registry.pinballEnemies.components[i].attackTimer, ms);
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

                Entity projectile_ball = createBall(r, spawnPos, pinball.pinBallSize);
                createNewRectangleTiedToEntity(projectile_ball, pinball.pinBallSize, pinball.pinBallSize, registry.motions.get(projectile_ball).position, true, 1);

                TemporaryProjectile temp;
                temp.hitsLeft = 1;
                temp.bonusBall = false;
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
    if (registry.pinballEnemies.entities.size() == 0) {
        // exit_combat();
        // GameSceneState = 0;
    } else {
        if (particleSpawnTimer > 0.2f) {
            float pinballRadius = registry.pinBalls.components[0].pinBallSize;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> distRadius(0, pinballRadius);
            std::uniform_real_distribution<> distAngle(0.f, 2.f * M_PI);
            std::normal_distribution<> distLifeSpan(1.0f, 0.2f);
            std::normal_distribution<> distVel(0.f, 1.f);
            int numParticles = registry.pinBalls.components[0].pinBallSize*registry.pinBalls.components[0].pinBallSize/10;
            for (Entity entity: registry.balls.entities) {
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
            particleSpawnTimer=0.f;
        } else {
            particleSpawnTimer+=elapsed_ms_since_last_update;
        }

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

    update_swarm_motion();

    updateTimers(elapsed_ms_since_last_update);
    stepEnemyAttack();

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

    // Resetting game
    if (action == GLFW_RELEASE && key == GLFW_KEY_R)
    {
        exit_combat();
        world->restart_game();
    }

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

    if (action == GLFW_RELEASE && key == GLFW_KEY_F)
    {
        pinballDash();
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_K)
    {
        PinBall& pinBall = registry.pinBalls.components[0];
        Entity projectile_ball = createBall(renderer, { 400, 400 }, pinBall.pinBallSize);
        createNewRectangleTiedToEntity(projectile_ball, pinBall.pinBallSize, pinBall.pinBallSize, registry.motions.get(projectile_ball).position, true, 1);

        TemporaryProjectile temp;
        temp.hitsLeft = 2;
        temp.bonusBall = false;
        DamageToPlayer d;
        d.damage = 20.0f;
        DamageToEnemy d2;
        d2.damage = registry.pinBalls.components[0].pinBallDamage * 2.0f;

        registry.attackPower.emplace(projectile_ball, d2);
        registry.damages.emplace(projectile_ball, d);
        registry.temporaryProjectiles.emplace(projectile_ball, temp);

    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_Q)
    {
        if (registry.pinballPlayerStatus.components[0].comboCounter >= 5) {

            registry.pinballPlayerStatus.components[0].comboCounter - 5;
            printf("Spawned bonus ball! Combo count=%i ", registry.pinballPlayerStatus.components[0].comboCounter);

            vec2 spawnPos = vec2(registry.physObjs.get(registry.playerFlippers.entities[0]).center.x,
                registry.physObjs.get(registry.playerFlippers.entities[0]).center.y - 50.0f);

            PinBall& pinBall = registry.pinBalls.components[0];
            Entity projectile_ball = createBall(renderer, spawnPos, pinBall.pinBallSize);
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
}

void PinballSystem::spawn_swarm(vec2 boundary) {


    for (int i = 0; i < SWARM_SIZE; i++) {

        float angle = 2 * M_PI * i / SWARM_SIZE;
        float pos_x = S_CENTER_X + S_RADIUS * cos(angle);
        float pos_y = S_CENTER_Y + S_RADIUS * sin(angle);

        Entity swarmEnemy = createSwarmEnemy(renderer, vec2(pos_x, pos_y));
        registry.colors.insert(swarmEnemy, {0, 0, 1});

        Motion& motion = registry.motions.get(swarmEnemy);
//        motion.angle = angle;

        float vel_x = - S_SPEED * cos(angle);
        float vel_y = - S_SPEED * sin(angle);

        motion.velocity = {vel_x, vel_y};

    }

    Entity swarmKing = createPinBallEnemy(renderer, vec2(525, 200), boundary, 0.5, 1, 5000.0f, 0.5);
    registry.swarmKing.insert(swarmKing,{});

    registry.colors.insert(swarmKing, { 0, 1, 0 });
}


void PinballSystem::update_swarm_motion() {

    Entity swarmKing = registry.swarmKing.entities[0];

    for (Entity entity: registry.swarmEnemies.entities) {
        Motion& motion = registry.motions.get(entity);

        vec2 rule_sum = rule1(entity, 50) + rule2(entity, 10) + rule3(entity, 14)
                + rule4(swarmKing, entity, 30);
        // scaling so that it doesn't go too fast
        float scaling = 0.01;
        rule_sum = {scaling * rule_sum.x, scaling * rule_sum.y};

        motion.velocity += rule_sum;
        motion.position += motion.velocity;
    }
}

vec2 PinballSystem::rule1(Entity b_j, float coherence = 100) {
    vec2 pc = {0.f, 0.f};

    // find perceived center
    for (Entity b: registry.swarmEnemies.entities) {
        if (b != b_j) {
            Motion m = registry.motions.get(b);
            pc += m.position;
        }
    }

    // move boid 1% of the way towards the center
    auto swarmSize = static_cast<float>(registry.swarmEnemies.size()) - 1;
    pc = {pc.x / swarmSize, pc.y / swarmSize};
    Motion m_j = registry.motions.get(b_j);
    vec2 diff = pc - m_j.position;
    return {diff.x / coherence, diff.y / coherence};
}

vec2 PinballSystem::rule2(Entity b_j, float separation = 50) {
    vec2 c = {0, 0};

    Motion m_j = registry.motions.get(b_j);

    for (Entity b: registry.swarmEnemies.entities) {
        if (b != b_j) {
            Motion m = registry.motions.get(b);

            vec2 diff = m.position - m_j.position;
            float length = sqrt(diff.x * diff.x  + diff.y * diff.y);

            if (length < separation) {
                c = c - (m.position - m_j.position);
            }
        }
    }

    return c;
}

vec2 PinballSystem::rule3(Entity b_j, float alignment = 8) {
    vec2 pv = {0.f ,0.f};

    for (Entity b: registry.swarmEnemies.entities) {
        if (b != b_j) {
            Motion m = registry.motions.get(b);
            pv += m.velocity;
        }
    }

    auto swarmSize = static_cast<float>(registry.swarmEnemies.size()) - 1;
    pv = {pv.x / swarmSize, pv.y / swarmSize};

    Motion m_j = registry.motions.get(b_j);
    pv = pv - m_j.velocity;
    pv = {pv.x / alignment, pv.y / alignment};

    return pv;
}

vec2 PinballSystem::rule4(Entity swarmKing, Entity b_j, float strength) {

    Motion king_m = registry.motions.get(swarmKing);
    Motion b_m = registry.motions.get(b_j);

    vec2 diff = king_m.position - b_m.position;

    return {diff.x / strength, diff.y / strength};
}

void PinballSystem::restart() {
    // int w, h;
    // glfwGetWindowSize(window, &w, &h);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    createPinballRoom(renderer, { 600, 400 }, window);
    r = renderer;
    vec2 boundary = {260 + 70, 800 - 70};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution1(boundary.x, boundary.y);
    std::uniform_real_distribution<float> distribution2(0.f, 1.f);

    spawn_swarm(boundary);

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
    Entity player_ball = createBall(renderer, { 400, 400 }, pinBall.pinBallSize);
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

//    Entity pinballenemyMain = createPinBallEnemy(renderer, vec2(525,180), boundary,2.0f, 0, 3000.0f);

//    registry.colors.insert(pinballenemyMain, { distribution2(gen), distribution2(gen), distribution2(gen) });


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


