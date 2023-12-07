// Header
#include "swarm_system.hpp"
#include "world_init.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"


const size_t SWARM_SIZE = 50;
const int S_CENTER_X = 525;
const int S_CENTER_Y = 300;
const int S_RADIUS = 200;
const int S_SPEED = 1;

const float COHERENCE = 8;
const float SEPARATION = 10;
const float ALIGNMENT = 20;
const float LEADER = 10;
const float LEADER_SEP = 80;

SwarmSystem::SwarmSystem(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
}

SwarmSystem::~SwarmSystem() {

}
void SwarmSystem::spawn_swarm(vec2 boundary) {


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

    Entity swarmKing = createPinBallEnemy(renderer, vec2(525, 300), boundary, 0.5, 0, 5000.0f, 0.5);
    registry.swarmKing.insert(swarmKing,{});

    PinBallEnemy& pinballEnemy = registry.pinballEnemies.get(swarmKing);
    pinballEnemy.maxHealth = 300.f;
    pinballEnemy.currentHealth = 300.f;

    registry.colors.insert(swarmKing, { 0, 1, 0 });
}

bool SwarmSystem::collides(Entity b_j) {
    Entity mainBall;
    for (int i = 0; i < registry.balls.components.size(); i++) {
        if (registry.balls.components[i].isMainBall) {
            mainBall = registry.balls.entities[i];
        }
    }

    Motion &ball_motion = registry.motions.get(mainBall);
    Motion &boid_motion = registry.motions.get(b_j);



    return false;
}

void SwarmSystem::handle_collision(Entity b_j) {

}


void SwarmSystem::update_swarm_motion() {

    Entity swarmKing = registry.swarmKing.entities[0];

    for (Entity entity: registry.swarmEnemies.entities) {
        Motion& motion = registry.motions.get(entity);

        vec2 rule_sum = rule1(entity, COHERENCE)
                + rule2(entity, SEPARATION)
                + rule3(entity, ALIGNMENT)
                + rule4(swarmKing, entity, LEADER)
                + rule5(swarmKing, entity, LEADER_SEP);

        // scaling so that it doesn't go too fast
        float scaling = 0.01;
        rule_sum = {scaling * rule_sum.x, scaling * rule_sum.y};

        motion.velocity += rule_sum;
        motion.position += motion.velocity;
    }
}

vec2 SwarmSystem::rule1(Entity b_j, float coherence = 100) {
    vec2 pc = {0.f, 0.f};

    // find perceived center
    for (Entity b: registry.swarmEnemies.entities) {
        if (b != b_j) {
            Motion m = registry.motions.get(b);
            pc += m.position;
        }
    }

    // move boid towards the center
    auto swarmSize = static_cast<float>(registry.swarmEnemies.size()) - 1;
    pc = {pc.x / swarmSize, pc.y / swarmSize};
    Motion m_j = registry.motions.get(b_j);
    vec2 diff = pc - m_j.position;
    return {diff.x / coherence, diff.y / coherence};
}

vec2 SwarmSystem::rule5(Entity swarmKing, Entity b_j, float leader_separation = 30) {
    vec2 king_pos = registry.motions.get(swarmKing).position;
    vec2 m_pos = registry.motions.get(b_j).position;
    vec2 diff = king_pos - m_pos;
    float dist = sqrt(diff.x * diff.x + diff.y * diff.y);

    if (dist < leader_separation) {
        return - diff;
    } else {
        return {0, 0};
    }
}

vec2 SwarmSystem::rule2(Entity b_j, float separation = 50) {
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

vec2 SwarmSystem::rule3(Entity b_j, float alignment = 8) {
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

vec2 SwarmSystem::rule4(Entity swarmKing, Entity b_j, float strength) {

    Motion king_m = registry.motions.get(swarmKing);
    Motion b_m = registry.motions.get(b_j);

    vec2 diff = king_m.position - b_m.position;

    return {diff.x / strength, diff.y / strength};
}