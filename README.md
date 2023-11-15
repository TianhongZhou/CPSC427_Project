# Team18_Pinball_Luminary

## Entry points to features

- ### Loading and rendering of textured geometry with correct blending and consistent asset drawing order:
	- Functions createXXX in world_init.cpp
	- mesh_paths in render_system.hpp
	- texture_paths in render_system.hpp
- ### Working basic 2D transformations:
    - drawTexturedMesh in render_system.cpp
- ### Key-frame/state interpolation (smooth movement from point A to point B in Cartesian or angle space)
    - struct PositionKeyFrame in components.hpp
    - positionKeyFrames in tiny_ecs_registry.hpp
    - step_world in ai_system.cpp
- ### Keyboard/mouse control of at least one character sprite. This can include changes in the set of rendered objects, object geometry, position, orientation, textures, colors, and other attributes
    - on_key and on_mouse_click in world_system.cpp
- ### Randomized or hard-coded action of one or more characters/assets
    - step_world in world_system.cpp
- ### Well defined game space boundaries
    - step_world in world_system.cpp
    - step_world in physics_system.cpp
- ### Correct collision processing including basic detection and avoidance. (preventing obvious penetrations)
    - detectAndSolveAllCollisions in physics_system.cpp
    - handle_collisions_world in world_system.cpp
- ### Basic physics
    - createNewRectangleTiedToEntity in world_init.cpp
    - physObj component and various sub-components in components.hpp
- ### Simple rendering effects
    - post.fs.glsl
    - post.vs.glsl
    - draw_world in render_system.cpp
- ### Animation: Implement sprite sheet animation or an equivalent animation system
    - struct SpriteSheet in components.hpp
    - spriteSheets in tiny_ecs_registry.hpp
    - sprite_sheet_path in common.hpp
    - createRoomEnemy in world_init.cpp
    - on_mouse_click in world_system.cpp
    - on_key in world_system.cpp
    - drawTexturedMesh in render_system.cpp
    - textured.vs.glsl
    - textured.fs.glsl
- ### Assets: Introduce new sprite and background assets (background music & attacking sound)
    - create_window in world_system.cpp
    - on_mouse_click in world_system.cpp
    - on_key in world_system.cpp and step_world in ai_system.cpp (projectiles shot by player/enemy)
- ### 2D Dynamic Shadow
    - draw_world in render_system.cpp
    - struct Light in components.hpp
    - lights in tiny_ecs_registry.hpp
    - shadow.png in textures
    - drawShadow in render_system.cpp
    - createShadow in world_init.cpp
- ### Help
    - draw_world in render_system.cpp
- ### Playability
    - #### MainWorld enemy AI system
   	    - stepWorld in ai_system.cpp
    - #### MainWorld enemy periodic generation
   	    - stepWorld in world_system.cpp
    - #### MainWorld player-enemy collision and combat trigering
   	    - stepWorld in world_system.cpp
   	    - restart in world_system.cpp
    - #### Health system
	    - createPinBallEnemyHealth in world_init.cpp
	    - createPinBallEnemy in world_init.cpp
	    - step in ai_system.cpp
    - #### Drop (buff) system
        - struct PinBall in components.hpp (store current size and damage, add to player of dungeon)
        - struct DropBuff in components.hpp
        - DropBuffAdd in world_system.hpp
        - GenerateDropBuff in world_system.hpp
        - createDropBuff in world_init.cpp
        - random generate drop in createRoom in world_init.cpp

## Actual development progress
The original development plan for the week of Sept. 31 and Oct. 8 is as following:

- Week: Sept 31 - Skeletal Game
    - Layout basic class hierarchy/structure
    - Basic design for pinball game
    - Basic collision detection for pinball game
    - Basic design for dungeon
    - Basic movement across the dungeon
- Week: Oct 8
    - Basic scene for dungeon completed (game space boundaries, some actions for character/assets)
    - Basic collision detection for dungeon (at least for boundaries)
    - Basic scene change 
- Week: Oct 15
    - Basic dungeon logic
    - Basic dungeon progression system (cards the player chooses after combat or collect items)
    - Render some art assets with game
- Week: Oct 22 - Minimal Playability
    - (Smooth) enemy collision and scene change
    - Tutorial design
    - Different types of dungeon rooms
    - Sprite sheet animation
    - Integrate more art assets
- Week: Oct 29
    - Implement a basic tutorial
    - Basic randomized dungeon generation logic 
    - Improved dungeon progression system
    - Scale difficulty of combat with dungeon progression

Basic dungeon logic is done on Oct. 30 instead in the week of Oct. 15. Did not implement basic dungeon progression system. Rendering is done in the week of Oct. 22 instead of Oct. 15. Did not implement different types of dungeon rooms. Tutorial design is done in the week of Oct. 29 instead of Oct 22. Except for the ones mentioned above, all other points listed are finished on time. The discrepancies between the development plan and actual development progress is also listed.

References:

Physics 
https://www.youtube.com/watch?v=lS_qeBy3aQI, 
http://programmerart.weebly.com/separating-axis-theorem.html, 
https://www.gamedev.net/tutorials/programming/math-and-physics/a-verlet-based-approach-for-2d-game-physics-r2714/
