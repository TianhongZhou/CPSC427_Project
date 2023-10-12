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
-   - createNewRectangleTiedToEntity in world_init.cpp
-   - physObj component and various sub-components in components.hpp
- ### Simple rendering effects
