#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

/**
 * @brief Represents a 3D camera for rendering with configurable parameters.
 *
 * Provides camera properties like near/far clipping planes, aspect ratio,
 * field of view, and camera orientation. Offers functions for movement,
 * rotation, and generating view/projection matrices.
 */
struct camera
{
    /// Near clipping plane distance.
    float near_clip = 0.001f;

    /// Far clipping plane distance.
    float far_clip = 100.0f;

    /// Aspect ratio (width / height) of the viewport.
    float aspect_ratio = 4.0f / 3.0f;

    /// Vertical field of view in radians.
    float field_of_view = 1.0f;

    /// Position of the camera in world space.
    glm::vec3 camera_position = glm::vec3(5.0f, 0.0f, 0.0f);

    /// Target point the camera is looking at.
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);

    /// Up direction of the camera.
    glm::vec3 camera_up_vector = glm::vec3(0.0f, 0.0f, 1.0f);

    /// Rotation in x- and y- screen space directions.
    glm::vec2 rotation = glm::vec2(0.0f, 0.0f);

    /// Distance from camera to target.
    float get_target_distance();

    /**
     * @brief Sets the distance from the camera to its target.
     * @param distance Distance to target.
     */
    void set_target_distance(float distance);

    /**
     * @brief Moves the camera target by a given offset.
     * @param delta Translation vector.
     */
    void translate_target(glm::vec3 delta);

    /**
     * @brief Rotates the camera around its target by given angles.
     * @param rotation Rotation vector (x, y) in screen space.
     */
    void rotate_around_target(glm::vec2 rotation);

    /// Computes the combined view-projection matrix.
    glm::mat4 get_view_projection_matrix();

    /// Computes the view matrix for this camera.
    glm::mat4 get_view_matrix();

    /// Computes the projection matrix for this camera.
    glm::mat4 get_projection_matrix();

    /**
     * @brief Defines camera rotation behavior.
     */
    enum {
        free_move,       ///< Camera moves freely without a pivot
        free_around_axis ///< Camera rotates around a fixed axis
    } rotation_pivot_type = free_around_axis;

    /// Whether the camera X axis should be flipped in "free_around_axis" mode.
    float flip_x_axis = 1.0f;

    /// Size of the viewport in pixels (width, height).
    glm::vec2 viewport_size = glm::vec2(1920.0f, 1080.0f);
};

#endif // CAMERA_H
