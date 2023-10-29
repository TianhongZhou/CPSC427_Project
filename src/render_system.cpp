// internal
#include "render_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "world_system.hpp"

// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void RenderSystem::drawTexturedMesh(Entity entity,
                                    const mat3 &projection) {
    Motion &motion = registry.motions.get(entity);
    // Transformation code, see Rendering and Transformation in the template
    // specification for more info Incrementally updates transformation matrix,
    // thus ORDER IS IMPORTANT
    Transform transform;
    transform.translate(motion.position);
    transform.rotate(motion.angle);
    transform.scale(motion.scale);

    assert(registry.renderRequests.has(entity));
    const RenderRequest &render_request = registry.renderRequests.get(entity);

    const GLuint used_effect_enum = (GLuint) render_request.used_effect;
    assert(used_effect_enum != (GLuint) EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint) effects[used_effect_enum];

    // Setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint) render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint) render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // Input data location as in the vertex buffer
    if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
        gl_has_errors();
        assert(in_texcoord_loc >= 0);

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(TexturedVertex), (void *) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(
                in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                (void *) sizeof(
                        vec3)); // note the stride to skip the preceeding vertex position

        // Enabling and binding texture to slot 0
        glActiveTexture(GL_TEXTURE0);
        gl_has_errors();

        assert(registry.renderRequests.has(entity));
        GLuint texture_id =
                texture_gl_handles[(GLuint) registry.renderRequests.get(entity).used_texture];


        // enter combat?
        GLint enter_combat_uloc = glGetUniformLocation(program, "enter_combat");
        assert(enter_combat_uloc >= 0);
        glUniform1i(enter_combat_uloc, registry.enterCombatTimer.has(entity));

        if (registry.spriteSheets.has(entity)) {
            SpriteSheet &spriteSheet = registry.spriteSheets.get(entity);
            glUniform2f(glGetUniformLocation(program, "spritesheetSize"), (float) spriteSheet.spriteSheetWidth,
                        (float) spriteSheet.spriteSheetHeight);
            float currentFrameX = spriteSheet.currentFrame % spriteSheet.spriteSheetWidth;
            float currentFrameY = spriteSheet.currentFrame / spriteSheet.spriteSheetWidth;
            glUniform2f(glGetUniformLocation(program, "currentFrame"), currentFrameX, currentFrameY);
            glUniform1i(glGetUniformLocation(program, "xFlip"), spriteSheet.xFlip ? 1 : 0);

            spriteSheet.frameAccumulator += spriteSheet.frameIncrement;
            if (spriteSheet.frameAccumulator >= 1.0f) {
                spriteSheet.currentFrame++;
                spriteSheet.frameAccumulator -= 1.0f;
            }
            if (spriteSheet.currentFrame >= spriteSheet.totalFrames) {
                if (spriteSheet.loop) {
                    spriteSheet.currentFrame = 0;
                } else {
                    RenderRequest &renderRequest = registry.renderRequests.get(entity);
                    renderRequest.used_texture = spriteSheet.origin;
                    registry.spriteSheets.remove(entity);
                }
            }
        } else {
            glUniform2f(glGetUniformLocation(program, "spritesheetSize"), 1.0, 1.0);
            glUniform2f(glGetUniformLocation(program, "currentFrame"), 0.0, 0.0);
            bool shouldxFlipThisEntity = motion.velocity.x < 0;
            glUniform1i(glGetUniformLocation(program, "xFlip"), shouldxFlipThisEntity ? 1 : 0);
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);
        gl_has_errors();
    } else if (render_request.used_effect == EFFECT_ASSET_ID::SALMON ||
               render_request.used_effect == EFFECT_ASSET_ID::PEBBLE) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(ColoredVertex), (void *) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(ColoredVertex), (void *) sizeof(vec3));
        gl_has_errors();

        if (render_request.used_effect == EFFECT_ASSET_ID::SALMON) {
            // HighLight Enemy
            GLint highlight_uloc = glGetUniformLocation(program, "highlight");
            assert(highlight_uloc >= 0);
            const int li = registry.highLightEnemies.has(entity) ? 1 : 0;
            glUniform1i(highlight_uloc, li);
            gl_has_errors();
        }
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
    glUniform3fv(color_uloc, 1, (float *) &color);
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);
    // GLsizei num_triangles = num_indices / 3;

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *) &transform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *) &projection);
    gl_has_errors();
    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

void RenderSystem::drawShadow(Entity entity, const mat3 &projection, const float angleRadians, const vec2 scale) {
    Motion &motion = registry.motions.get(entity);

    assert(registry.renderRequests.has(entity));
    const RenderRequest &render_request = registry.renderRequests.get(entity);

    GLuint textureID = (GLuint) registry.renderRequests.get(entity).used_texture;
    GLint width, height;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    vec2 texture_size = vec2((int) width, (int) height);

    Transform transform;
    transform.translate(motion.position);
    transform.translate(vec2(0, height / 2.0f));
    transform.rotate(angleRadians);
    texture_size = vec2(1.0f, 3.0f) * texture_size;
    texture_size = scale * texture_size;
    transform.translate(vec2(0, -texture_size.y));
    transform.scale(motion.scale);
    transform.scale(vec2(1.0f, 3.0f));
    transform.scale(scale);

    const GLuint used_effect_enum = (GLuint) render_request.used_effect;
    assert(used_effect_enum != (GLuint) EFFECT_ASSET_ID::EFFECT_COUNT);
    const GLuint program = (GLuint) effects[used_effect_enum];

    // Setting shaders
    glUseProgram(program);
    gl_has_errors();

    assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
    const GLuint vbo = vertex_buffers[(GLuint) render_request.used_geometry];
    const GLuint ibo = index_buffers[(GLuint) render_request.used_geometry];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // Input data location as in the vertex buffer
    if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED) {
        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
        gl_has_errors();
        assert(in_texcoord_loc >= 0);

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(TexturedVertex), (void *) 0);
        gl_has_errors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(
                in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
                (void *) sizeof(
                        vec3)); // note the stride to skip the preceeding vertex position

        // Enabling and binding texture to slot 0
        glActiveTexture(GL_TEXTURE0);
        gl_has_errors();

        assert(registry.renderRequests.has(entity));
        //GLuint texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::SHADOW];
        GLuint texture_id = texture_gl_handles[(GLuint) registry.renderRequests.get(entity).used_texture];

        GLint enter_combat_uloc = glGetUniformLocation(program, "enter_combat");
        assert(enter_combat_uloc >= 0);
        glUniform1i(enter_combat_uloc, registry.enterCombatTimer.has(entity));

        if (registry.spriteSheets.has(entity)) {
            SpriteSheet &spriteSheet = registry.spriteSheets.get(entity);
            glUniform2f(glGetUniformLocation(program, "spritesheetSize"), (float) spriteSheet.spriteSheetWidth,
                        (float) spriteSheet.spriteSheetHeight);
            float currentFrameX = spriteSheet.currentFrame % spriteSheet.spriteSheetWidth;
            float currentFrameY = spriteSheet.currentFrame / spriteSheet.spriteSheetWidth;
            glUniform2f(glGetUniformLocation(program, "currentFrame"), currentFrameX, currentFrameY);
            glUniform1i(glGetUniformLocation(program, "xFlip"), spriteSheet.xFlip ? 1 : 0);

            spriteSheet.frameAccumulator += spriteSheet.frameIncrement;
            if (spriteSheet.frameAccumulator >= 1.0f) {
                spriteSheet.currentFrame++;
                spriteSheet.frameAccumulator -= 1.0f;
            }
            if (spriteSheet.currentFrame >= spriteSheet.totalFrames) {
                if (spriteSheet.loop) {
                    spriteSheet.currentFrame = 0;
                } else {
                    RenderRequest &renderRequest = registry.renderRequests.get(entity);
                    renderRequest.used_texture = spriteSheet.origin;
                    registry.spriteSheets.remove(entity);
                }
            }
        } else {
            glUniform2f(glGetUniformLocation(program, "spritesheetSize"), 1.0, 1.0);
            glUniform2f(glGetUniformLocation(program, "currentFrame"), 0.0, 0.0);
            bool shouldxFlipThisEntity = motion.velocity.x < 0;
            glUniform1i(glGetUniformLocation(program, "xFlip"), shouldxFlipThisEntity ? 1 : 0);
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);
        gl_has_errors();
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    const vec3 color = vec3(0);
    glUniform3fv(color_uloc, 1, (float *) &color);
    gl_has_errors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();

    GLsizei num_indices = size / sizeof(uint16_t);
    // GLsizei num_triangles = num_indices / 3;

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *) &transform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *) &projection);
    gl_has_errors();
    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}


// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen() {
    // Setting shaders
    // get the water texture, sprite mesh, and program
    glUseProgram(effects[(GLuint) EFFECT_ASSET_ID::WATER]);
    gl_has_errors();
    // Clearing backbuffer
    int w, h;
    glfwGetFramebufferSize(window, &w,
                           &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(1.f, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl_has_errors();
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Draw the screen texture on the quad geometry
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint) GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
    glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            index_buffers[(GLuint) GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
    // indices to the bound GL_ARRAY_BUFFER
    gl_has_errors();
    const GLuint water_program = effects[(GLuint) EFFECT_ASSET_ID::WATER];
    // Set clock
    GLuint time_uloc = glGetUniformLocation(water_program, "time");
    GLuint dead_timer_uloc = glGetUniformLocation(water_program, "screen_darken_factor");
    glUniform1f(time_uloc, (float) (glfwGetTime() * 10.0f));
    ScreenState &screen = registry.screenStates.get(screen_state_entity);
    glUniform1f(dead_timer_uloc, screen.screen_darken_factor);
    gl_has_errors();
    // Set the vertex position and vertex texture coordinates (both stored in the
    // same VBO)
    GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *) 0);
    gl_has_errors();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    gl_has_errors();

    // Flicker
    GLint flicker_uloc = glGetUniformLocation(water_program, "flicker");
    assert(flicker_uloc >= 0);
    const int li = registry.enterCombatTimer.size() > 0 ? 1 : 0;
    glUniform1i(flicker_uloc, li);
    gl_has_errors();

    // Draw
    glDrawElements(
            GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
            nullptr); // one triangle = 3 vertices; nullptr indicates that there is
    // no offset from the bound index buffer
    gl_has_errors();
}


mat3 RenderSystem::createProjectionMatrix() {
    // Fake projection matrix, scales with respect to window coordinates
    float left = 0.f;
    float top = 0.f;

    gl_has_errors();
    float right = (float) window_width_px;
    float bottom = (float) window_height_px;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    return {{sx,  0.f, 0.f},
            {0.f, sy,  0.f},
            {tx,  ty,  1.f}};
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw_world(bool &tutorial_open) {

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool p_open = tutorial_open;

    if (p_open) {
        ImGui::Begin("Tutorial", &p_open);
        ImGui::TextWrapped("While this box is open the world is paused. \n"
                           "This box should contain the basic tutorial instructions.");
        ImGui::End();
    }

    if (!p_open) {
        tutorial_open = p_open;
    }

    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(window, &w,
                           &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    gl_has_errors();
    // Clearing backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0.00001, 10);
    glClearColor(0, 0, 0, 1.0);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
    // and alpha blending, one would have to sort
    // sprites back to front
    gl_has_errors();

    std::vector<Light> lights = {};

    for (Entity entity: registry.lights.entities) {
        if (!registry.lights.has(entity)) {
            continue;
        }

        RenderRequest &renderRequest = registry.renderRequests.get(entity);
        Motion &motion = registry.motions.get(entity);
        Light &light = registry.lights.get(entity);
        if ((renderRequest.used_texture == TEXTURE_ASSET_ID::PLAYER) ||
            (renderRequest.used_texture == TEXTURE_ASSET_ID::PLAYERATTACKSPRITESHEET) ||
            (renderRequest.used_texture == TEXTURE_ASSET_ID::PLAYERWALKSPRITESHEET)) {
            light.screenPosition = vec2(motion.position.x / w, (h - motion.position.y) / h);
            light.haloRadius = 0.2f;
            light.lightColor = vec3(1.0f, 1.0f, 1.0f);
            light.haloSoftness = 0.05f;
            light.priority = 2;
        }
        lights.push_back(light);
    }

    mat3 projection_2D = createProjectionMatrix();

    // Draw all textured meshes that have a position and size component
    for (Entity entity: registry.renderRequests.entities) {
        if (!registry.motions.has(entity))
            continue;

        if (GameSceneState == 0 && registry.combat.has(entity)) {
            continue;
        }

        if (GameSceneState == 1 && !registry.combat.has(entity)) {
            continue;
        }


        if (GameSceneState == 0) {

            RenderRequest &renderRequest = registry.renderRequests.get(entity);
            if (renderRequest.used_texture == TEXTURE_ASSET_ID::SHADOW) {
                continue;
            }

            Motion &motion = registry.motions.get(entity);

            for (Light light: lights) {
                glm::vec2 lightPosition = light.screenPosition;
                glm::vec2 entityPosition = vec2(motion.position.x / w, (h - motion.position.y) / h);
                glm::vec2 shadowOffset = glm::normalize(lightPosition - entityPosition) * 10.0f;
                glm::vec4 shadowColor = glm::vec4(1.0, 0.0, 0.0, 1.0);

                if (entityPosition == lightPosition) {
                    continue;
                }
                if (glm::length(entityPosition - lightPosition) >= (light.haloRadius + light.haloSoftness)) {
                    continue;
                }

                float dy = entityPosition.y - lightPosition.y;
                float dx = entityPosition.x - lightPosition.x;
                float angle = atan2(dy, dx);

                vec2 scale = vec2(1.0f,
                                  glm::length(entityPosition - lightPosition) /
                                  (light.haloRadius + light.haloSoftness));

                //if (scale.y < 0.31)
                //{
                //	scale.y = 0.31;
                //}
                drawShadow(entity, projection_2D, M_PI / 2 - angle, scale);
            }
        }

        drawTexturedMesh(entity, projection_2D);
    }

    // Truely render to the screen
    drawToScreen();


    if (GameSceneState == 0) {
        // Make the screen black
        float quadVertices[] = {
                -1.0f, 1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,

                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f
        };
        unsigned int quadVAO, quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
        const GLuint post_program = effects[(GLuint) EFFECT_ASSET_ID::POST];

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(post_program);

        glUniform1i(glGetUniformLocation(post_program, "screenTexture"), 0);

        // Draw lights
        draw_lights(post_program, lights, (float) h / (float) w);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    gl_has_errors();

}


void RenderSystem::draw_lights(GLuint post_program, std::vector<Light> lights, float aspectRatio) {
    std::sort(lights.begin(), lights.end(), [](const Light &a, const Light &b) {
        return a.priority > b.priority;
    });

    std::vector<glm::vec2> lightPositions = {};
    std::vector<float> lightRadii = {};
    std::vector<float> lightSoftnesses = {};
    std::vector<glm::vec3> lightColors = {};
    std::vector<int> prioritys = {};

    for (int i = 0; i < lights.size(); ++i) {
        lightPositions.push_back(lights[i].screenPosition);
        lightRadii.push_back(lights[i].haloRadius);
        lightSoftnesses.push_back(lights[i].haloSoftness);
        lightColors.push_back(lights[i].lightColor);
    }

    for (int i = 0; i < lightPositions.size(); ++i) {
        glUniform2fv(glGetUniformLocation(post_program, ("positions[" + std::to_string(i) + "]").c_str()), 1,
                     glm::value_ptr(lightPositions[i]));
        glUniform1f(glGetUniformLocation(post_program, ("radii[" + std::to_string(i) + "]").c_str()), lightRadii[i]);
        glUniform1f(glGetUniformLocation(post_program, ("softnesses[" + std::to_string(i) + "]").c_str()),
                    lightSoftnesses[i]);
        glUniform3fv(glGetUniformLocation(post_program, ("colors[" + std::to_string(i) + "]").c_str()), 1,
                     glm::value_ptr(lightColors[i]));
    }

    glUniform1i(glGetUniformLocation(post_program, "numLights"), lightPositions.size());
    glUniform1f(glGetUniformLocation(post_program, "aspectRatio"), aspectRatio);
    gl_has_errors();
}
