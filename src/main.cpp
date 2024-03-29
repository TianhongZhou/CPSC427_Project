
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "pinball_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Game state global variables
extern int GameSceneState;
extern int InitCombat;
extern float Enter_combat_timer;

// Entry point
int main()
{
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	PinballSystem pinballSystem;
	AISystem ai_system;

	// Initializing window
	GLFWwindow *window = world_system.create_window();
	if (!window)
	{
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	render_system.init(window);
	world_system.init(&render_system);

	bool tutorial_open = false;

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over())
	{
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		if (GameSceneState == 0 && !tutorial_open)
		{
			if (Enter_combat_timer <= 0.f)
			{
				world_system.handle_collisions_world();
				physics_system.step_world(elapsed_ms);
			} else {
				Enter_combat_timer-=elapsed_ms;
				if (Enter_combat_timer <= 0.f) {
					GameSceneState = 1;
					InitCombat = 1;
				}
			}
			ai_system.step_world(elapsed_ms);
			world_system.step_world(elapsed_ms);
		}
		else if (GameSceneState == 1)
		{

			if (InitCombat)
			{
				pinballSystem.init(window, &render_system, &world_system);
				InitCombat = 0;
			}

			pinballSystem.step(elapsed_ms);
			if (GameSceneState == 0)
			{
				continue;
			}
			physics_system.step(elapsed_ms);
			ai_system.step(elapsed_ms);
//			pinballSystem.handle_collisions();
			render_system.draw_combat_scene();
		}
		else if (GameSceneState == -1 || GameSceneState == -2)
		{
			world_system.step_world(elapsed_ms);
		}

		if (GameSceneState == 0 || GameSceneState == -1 || GameSceneState == -2)
		{
			render_system.draw_world(tutorial_open);
		}
	}

	//    ImGui_ImplOpenGL3_Shutdown();
	//    ImGui_ImplGlfw_Shutdown();
	//    ImGui::DestroyContext();

	return EXIT_SUCCESS;
}
