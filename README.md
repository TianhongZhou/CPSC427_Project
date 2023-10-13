# Team18_Pinball_Luminary

## Entry points to features

- ### Loading and rendering of textured geometry with correct blending and consistent asset drawing order:
	- Functions createXXX in world_init.cpp
	- mesh_paths in render_system.hpp
	- texture_paths in render_system.hpp
- ### Working basic 2D transformations:
    - drawTexturedMesh in render_system.cpp
- ### Key-frame/state interpolation (smooth movement from point A to point B in Cartesian or angle space)
    - step and step_world in physics_system.cpp
- ### Keyboard/mouse control of at least one character sprite. This can include changes in the set of rendered objects, object geometry, position, orientation, textures, colors, and other attributes
    - on_key and on_mouse_click in world_system.cpp
- ### Randomized or hard-coded action of one or more characters/assets
- ### Well defined game space boundaries
- ### Correct collision processing including basic detection and avoidance. (preventing obvious penetrations)
- ### Basic physics
    - createNewRectangleTiedToEntity in world_init.cpp
    - physObj component and various sub-components in components.hpp
- ### Simple rendering effects
    -  	EnterCombatTimer in components.hpp
    -  	textured.fs.glsl

## Actual development progress
The original development plan for the week of Sept. 31 and Oct. 8 is as following:

Week: Sept 31 - Skeletal Game
- Layout basic class hierarchy/structure
- Basic design for pinball game
- Basic collision detection for pinball game
- Basic design for dungeon
- Basic movement across the dungeon
Week: Oct 8
- Basic scene for dungeon completed (game space boundaries, some actions for character/assets)
- Basic collision detection for dungeon (at least for boundaries)
- Basic scene change 

All points listed above are finished on time, while basic movement across the dungeon is done in the week of Oct. 8 instead of Sept. 31. All other parts follow the plan exactly. There are no discrepancies between the development plan and actual development progress.

