// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp, dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}



float MAX_Y_COORD = 750.0f;

float MIN_Y_COORD = 0.0f;

float MIN_X_COORD = 0.0f;


float FLIPPERHEIGHT = 700.f;

float FLIPPERDELTA = 30.f;


float MAX_X_COORD = 900.0f;

vec2 GRAV = { 0.f, 0.0003f };
//struct Vertex_Phys {
//	vec2 pos;
//	vec2 oldPos;
//
//	vec2 accel;
//
//};
//
//
//struct Edge {
//	Vertex_Phys* v1;
//	Vertex_Phys* v2;
//
//	physObj* parentObj;
//
//	float len;
//
//};
//
//
//struct physObj {
//
//	Vertex_Phys Vertices[8];
//	Edge Edges[13];
//
//
//	vec2 center;
//
//	int VertexCount;
//	int EdgesCount;
//
//
//};


struct CollisionEvent {
	vec2 Normal;
	float Depth;
	Edge* Edge;
	Vertex_Phys* Vertex;

	physObj* EdgeParent;
};







void updatePos(float dt, Vertex_Phys& v) {
	vec2 velocity = v.pos - v.oldPos;
	v.oldPos = v.pos;
	v.pos = v.pos + velocity + v.accel * dt * dt;
	v.accel = {};
}

void updateEdges(physObj& obj) {

	for (int i = 0; i < obj.EdgesCount; i++) {

		Edge& e = obj.Edges[i];

		vec2 v1v2 = obj.Vertices[e.v2].pos - obj.Vertices[e.v1].pos;

		float curr_len = length(v1v2);

		float diff = curr_len - e.len;

		vec2 direction = normalize(v1v2);

		obj.Vertices[e.v1].pos += direction * diff * 0.5f;
		obj.Vertices[e.v2].pos -= direction * diff * 0.5f;


	}


}








void projectObjToAxis(physObj& Obj, vec2 Axis, float& Max, float& Min) {

	float dp = dot(Axis, Obj.Vertices[0].pos);

	Max = Min = dp;

	for (int i = 1; i < Obj.VertexCount; i++) {
		dp = dot(Axis, Obj.Vertices[i].pos);

		Min = min(dp, Min);
		Max = max(dp, Max);

	}



}

vec2 findCenter(const physObj& a) {

	float cx = 0.f;
	float cy = 0.f;
	for (int i = 0; i < a.VertexCount; i++) {
		cx += a.Vertices[i].pos.x;
		cy += a.Vertices[i].pos.y;

	}


	return vec2(cx / a.VertexCount, cy / a.VertexCount);



}

void collisionResponse(CollisionEvent& e, bool otherObjMoveable, float otherObjCoef) {
	vec2 CollisionVector = e.Normal * e.Depth;

	int E1 = e.Edge->v1;
	int E2 = e.Edge->v2;

	float fac;
	if (abs(e.EdgeParent->Vertices[E1].pos.x - e.EdgeParent->Vertices[E2].pos.x) > abs(e.EdgeParent->Vertices[E1].pos.y - e.EdgeParent->Vertices[E2].pos.y))
		fac = (e.Vertex->pos.x - CollisionVector.x - e.EdgeParent->Vertices[E1].pos.x) / (e.EdgeParent->Vertices[E2].pos.x - e.EdgeParent->Vertices[E1].pos.x);
	else
		fac = (e.Vertex->pos.y - CollisionVector.y - e.EdgeParent->Vertices[E1].pos.y) / (e.EdgeParent->Vertices[E2].pos.y - e.EdgeParent->Vertices[E1].pos.y);

	float Lambda = 1.0f / (fac * fac + (1 - fac) * (1 - fac));


	if (e.EdgeParent->moveable) {

		e.EdgeParent->Vertices[E1].pos -= CollisionVector * (1 - fac) * 0.5f * Lambda * e.EdgeParent->knockbackCoef;
		e.EdgeParent->Vertices[E2].pos -= CollisionVector * fac * 0.5f * Lambda * e.EdgeParent->knockbackCoef;
	}


	if (otherObjMoveable) {
		e.Vertex->pos += CollisionVector * 0.5f * otherObjCoef;
	}
}


bool detectAndResloveCollision(physObj* a, physObj* b) {

	float minDist = 15000.0f;
	CollisionEvent event{};

	physObj* parent;

	for (int i = 0; i < a->EdgesCount + b->EdgesCount; i++) {
		Edge* currEdge;
		if (i < a->EdgesCount) {
			currEdge = &a->Edges[i];

			parent = a;
		}
		else {
			currEdge = &b->Edges[i - a->EdgesCount];

			parent = b;
		}

		vec2 Axis = vec2((*parent).Vertices[currEdge->v1].pos.y - (*parent).Vertices[currEdge->v2].pos.y,
			(*parent).Vertices[currEdge->v2].pos.x - (*parent).Vertices[currEdge->v1].pos.x);
		Axis = normalize(Axis);


		float MinA, MinB, MaxA, MaxB;

		projectObjToAxis(*a, Axis, MaxA, MinA);
		projectObjToAxis(*b, Axis, MaxB, MinB);
		float dist;

		if (MinA < MinB) {
			dist = MinB - MaxA;
		}
		else {
			dist = MinA - MaxB;
		}

		if (dist > 0.0f) {
			return false;

		}
		else if (abs(dist) < minDist) {
			minDist = abs(dist);


			event.Normal = Axis;
			event.Edge = currEdge;

			event.EdgeParent = parent;


			//emplace this new event

		}


	}

	event.Depth = minDist;


	if ((event.EdgeParent) != b) {
		physObj* temp = b;
		b = a;
		a = temp;
	}

	float check = dot(event.Normal, (a->center - b->center)); // questionable?

	int sign = (0.f < check) - (check < 0.f);

	if (sign != 1) {
		event.Normal = -event.Normal;
	}

	float smallestDist = 20000.0f;


	for (int i = 0; i < a->VertexCount; i++) {

		float distance = dot(event.Normal, (a->Vertices[i].pos - b->center)); //questionable


		if (distance < smallestDist) {
			smallestDist = distance;
			event.Vertex = &a->Vertices[i];
		}
	}


	collisionResponse(event, a->moveable, a->knockbackCoef);

	return true;


}




void accelerate(vec2 acc, Vertex_Phys& obj) {
	obj.accel += acc;

}



void updateAllEdges() {
	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj& obj = registry.physObjs.components[i];


		
		updateEdges(obj);
		
	}
}



void detectAndSolveAllCollisions() {

	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj* a = &registry.physObjs.components[i];


		for (uint i2 = 0; i2 < registry.physObjs.size(); i2++) {
			physObj* b = &registry.physObjs.components[i2];

			bool collide = false;

			if (a != b) {
				collide = detectAndResloveCollision(a, b);


			}

			// Handle Collision

			// If collided with an enemy
			Entity entity_a = registry.physObjs.entities[i];
			Entity entity_b = registry.physObjs.entities[i2];
			if (collide) {
				if (registry.pinballEnemies.has(entity_a) && !registry.pinballEnemies.has(entity_b) ||
					registry.pinballEnemies.has(entity_b) && !registry.pinballEnemies.has(entity_a)) {
					// remove enemy upon collision
					if (registry.pinballEnemies.has(entity_a)) {
						registry.remove_all_components_of(entity_a);
					}
					else {
						registry.remove_all_components_of(entity_b);
					}
					
				}
			}


		}



	}


}






void updateAllObjPos(float dt) {


	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj& obj = registry.physObjs.components[i];

		for (int i2 = 0; i2 < obj.VertexCount; i2++) {
			updatePos(dt, (obj.Vertices[i2]));
			//			printf("x=%f, y=%f\n", obj.Vertices[i2].pos.x, obj.Vertices[i2].pos.y);
		}

	}

}


void applyObjGrav() {
	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj& obj = registry.physObjs.components[i];

		if (obj.moveable) {

			for (int i2 = 0; i2 < obj.VertexCount; i2++) {
				accelerate(GRAV, (obj.Vertices[i2]));
			}

		}

	}
}


void updateAllCenters() {

	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj& obj = registry.physObjs.components[i];

		obj.center = findCenter(obj);

	}


}



void applyGlobalConstraints() {

	for (uint i = 0; i < registry.physObjs.size(); i++) {
		physObj& obj = registry.physObjs.components[i];

		for (int i2 = 0; i2 < obj.VertexCount; i2++) {
			if (obj.Vertices[i2].pos.y > MAX_Y_COORD) {
				obj.Vertices[i2].pos.y = MAX_Y_COORD;
			}


			if (obj.Vertices[i2].pos.y < MIN_Y_COORD) {
				obj.Vertices[i2].pos.y = MIN_Y_COORD;
			}

			if (obj.Vertices[i2].pos.x < MIN_X_COORD) {
				obj.Vertices[i2].pos.x = MIN_X_COORD;
			}


			if (obj.Vertices[i2].pos.x > MAX_X_COORD) {
				obj.Vertices[i2].pos.x = MAX_X_COORD;
			}

		}

	}


}






void updateAllMotionInfo() {


	for (uint i = 0; i < registry.physObjs.size(); i++) {
		Entity& obj = registry.physObjs.entities[i];

		float x = registry.physObjs.get(obj).Vertices[1].pos.x - registry.physObjs.get(obj).Vertices[0].pos.x;
		float y = registry.physObjs.get(obj).Vertices[1].pos.y - registry.physObjs.get(obj).Vertices[0].pos.y;

		registry.motions.get(obj).position = registry.physObjs.get(obj).center;

		//		printf("%f \n",registry.motions.get(obj).position.y);
		registry.motions.get(obj).angle = atan2(y, x);
		//		printf("angle = %f \n", atan2(y, x));
	}

}


void flipperConstraints() {

	for (uint i = 0; i < registry.playerFlippers.size(); i++) {
		Entity& flipper = registry.playerFlippers.entities[i];

		physObj& flipperPhys = registry.physObjs.get(flipper);

		if (flipperPhys.Vertices[1].pos.y != FLIPPERHEIGHT) {
			flipperPhys.Vertices[1].pos.y = FLIPPERHEIGHT;
		}

		if (flipperPhys.Vertices[3].pos.y > FLIPPERHEIGHT + FLIPPERDELTA) {
			flipperPhys.Vertices[3].pos.y = FLIPPERHEIGHT + FLIPPERDELTA;
		}

		if (flipperPhys.Vertices[3].pos.y < FLIPPERHEIGHT - FLIPPERDELTA) {
			flipperPhys.Vertices[3].pos.y = FLIPPERHEIGHT - FLIPPERDELTA;
		}

	}

	//if (registry.mousePosArray.size() != 0) {

	//	vec2 mouse_position = registry.mousePosArray.components[0].pos;

	//	if (mouse_position.x < 1000.0f && mouse_position.x > 0.0f && mouse_position.y < 1000.0f && mouse_position.y > 0.0f) {

	//	for (uint i = 0; i < registry.playerFlippers.size(); i++) {
	//		Entity& flipper = registry.playerFlippers.entities[i];

	//		physObj& flipperPhys = registry.physObjs.get(flipper);

	//		for (int k = 0; k < flipperPhys.VertexCount; k++) {
	//			flipperPhys.Vertices[k].pos.x = mouse_position.x;
	//			flipperPhys.Vertices[k].oldPos.x = mouse_position.x;
	//		}

	//	}
	//}

	//}

}


void update(float dt) {
	
	applyObjGrav();
	updateAllObjPos(dt);
	Entity& flipper = registry.playerFlippers.entities[0];

	flipperConstraints();
	physObj& flipperPhys = registry.physObjs.get(flipper);
	applyGlobalConstraints();
	updateAllEdges();
	flipperPhys = registry.physObjs.get(flipper);
	updateAllCenters();
	flipperPhys = registry.physObjs.get(flipper);
	detectAndSolveAllCollisions();
	flipperPhys = registry.physObjs.get(flipper);

	updateAllMotionInfo();
	
	flipperPhys = registry.physObjs.get(flipper);

}

void updateWithSubstep(float dt, float steps) {

	for (int i = 0; i < steps; i++) {
		update(dt / steps);
	}


}





void PhysicsSystem::step(float elapsed_ms)
{

	updateWithSubstep(elapsed_ms, 4.0f);

	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		Transform transform;
		transform.rotate(motion.angle);
		vec3 velocity = { motion.velocity.x, motion.velocity.y, 1.f };
		vec3 result = transform.mat * velocity;
		float x_velocity = result.x;
		float y_velocity = result.y;
		float x_position = motion.position.x + (x_velocity * step_seconds);
		float y_position = motion.position.y + (y_velocity * step_seconds);
		motion.position = { x_position, y_position };
	}

}


void PhysicsSystem::step_world(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_container.components[i];
		// Entity entity = motion_container.entities[i];
		float step_seconds = elapsed_ms / 1000.f;

		//2a, 2d: separate velocity components and calculate based on angle
		float v_x = motion.velocity.x * cos(motion.angle) + motion.velocity.y * sin(motion.angle);
		float v_y = motion.velocity.x * sin(motion.angle) + motion.velocity.y * cos(motion.angle);
		vec2 velocity = { v_x, v_y };

		motion.position += velocity * step_seconds;

		// Boundary check
		// Now we restrict everything, but we can choose what we want to restrict

		if (motion.position.x < 0) {
			motion.position.x = 0;
		}
		if (motion.position.y < 0) {
			motion.position.y = 0;
		}
		if (motion.position.x > window_width_px) {
			motion.position.x = window_width_px;
		}
		if (motion.position.y > window_height_px) {
			motion.position.y = window_height_px;
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		if (!registry.rooms.has(entity_i)) {
			// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
			for (uint j = i + 1; j < motion_container.components.size(); j++)
			{
				Motion& motion_j = motion_container.components[j];
				if (collides(motion_i, motion_j))
				{
					Entity entity_j = motion_container.entities[j];
					// Create a collisions event
					// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
					registry.collisions.emplace_with_duplicates(entity_j, entity_i);
				}
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}