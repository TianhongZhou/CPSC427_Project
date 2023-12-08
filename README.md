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
    - Physics-Based Animation: trailing effect using particles, particles have random diffusional motion like real life particles, and will float up due to buoyant.
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
    - #### MainWorld room generation
        - createRoom in world_init.cpp
        - Many other helper functions to generate rooms and its associated assets in world_init.cpp
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
    - #### Dash mechanic
    	- pinballDash() in pinball_system.cpp
    - #### Various timers (invincibility, attack, effects)
	    - countdown in pinball_system.cpp
	    - updateTimers in pinball_system.cpp
	    - timer fields in various components
    - #### Enemy attack
	    - stepEnemyAttack in in pinball_system.cpp
	    - temporary projectile component
    - #### Combo meter and bonus ball system
	    - PinballPlayerStatus component
	    - related code in on_key in pinball_system.cpp
    - #### Swarm AI and Enemy Type
   	    - SwarmKing and SwarmEnemy components
    	    - all functions in swarm_system.cpp
    
    
- ### Consistent game resolution
    - create_window in world_system.cpp
    - drawToScreen in render_system.cpp
    - draw_combat_scene in render_system.cpp
    - draw_world in render_system.cpp

- ### Reloadability
    - save_game and load_game in world_system.cpp
    - Used external library nlohmann/json (https://github.com/nlohmann/json)
    
- ### Basic Integrated Assets
    - createPinballRoom in world_init.cpp
    - createBall in world_init.cpp
    - createPinballWall in world_init.cpp
    - createPinballFlipper in world_init.cpp

- ### User Experience
- ## Tutorial
    - GameSceneState == -1 and -2 case
    - data/textures/start.png, tutorial.png
    - on_key and on_mouse_.. in world_system.cpp
- ## Report on the user testing you performed
    - googleDoc: https://docs.google.com/document/d/1RofeKl2MGerotKSOi44GetFGjieZlpuaCY7WcG6Qc-E/edit

- ### Normal Map
    - normal.fs.glsl
    - normal.vs.glsl
    - drawTexturedMesh in render_system.cpp
    
- ### Parallox Scrolling Background
    - createPinballRoom in world_init.cpp
    - Parallox component in component.hpp
    - Wrapping and shifting in textured.vs.glsl
    
- ### Shader effect in scene transition
    - filcker effect when entering combat in post.fs.glsl
    - shear, shake, blurr effect when exiting combat in water.vs.glsl and water.fs.glsl

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
- Week: Nov 5 - Playability
    - Procedural generation for dungeon
    - Advanced physics effects for pinball combat
    - Integrate all art assets needed for the game
    - Ensure robustness of game
- Week: Nov 12
    - Save/Load feature
    - More advanced physics effects or complex geometry, possibly more advanced enemies
    - Particle Effects
    - Flipper control change
    - New combat effects (high gravity and anti gravity)
    - Enemy attacks with temporary projectiles
    - Combo meter and bonus ball mechanic
    - Player Heath
    - Enemy and Player invincibility timer
    

Did not implement different types of dungeon rooms. Art assets are integrated in the week of Nov. 12 instead of Nov. 5. Did not implement more advanced enemies. Except for the ones mentioned above, all other points listed are finished on time. The discrepancies between the development plan and actual development progress is also listed.

References:

Physics 
https://www.youtube.com/watch?v=lS_qeBy3aQI, 
http://programmerart.weebly.com/separating-axis-theorem.html, 
https://www.gamedev.net/tutorials/programming/math-and-physics/a-verlet-based-approach-for-2d-game-physics-r2714/


free SFX from
https://pixabay.com/sound-effects/search/splat/
https://quicksounds.com/sound/104/minecraft-damage-oof
https://pixabay.com/sound-effects/search/punch/
https://www.myinstants.com/en/instant/hitmarkermp3/#google_vignette
https://pixabay.com/sound-effects/search/whoosh/
