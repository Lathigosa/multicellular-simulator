#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

struct camera
{
	float near_clip = 0.1f;
	float far_clip = 100.0f;
	float aspect_ratio = 4.0f / 3.0f;
	float field_of_view = 1.0f;

	glm::vec3 camera_position = glm::vec3(5.0f, 0.0f, 0.0f);
	glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camera_up_vector = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec2 rotation = glm::vec2(0.0f, 0.0f);				// Rotation in the x- and y- screen space directions.

	float get_target_distance();
	void set_target_distance(float distance);
	void translate_target(glm::vec3 delta);
	void rotate_around_target(glm::vec2 rotation);
	glm::mat4 get_view_projection_matrix();
	glm::mat4 get_view_matrix();
	glm::mat4 get_projection_matrix();

	enum {
		free_move,
		free_around_axis
	} rotation_pivot_type = free_around_axis;

	float flip_x_axis = 1.0;							// Boolean that determines whether the camera should be upside down in "free_around_axis" mode.
	
	glm::vec2 viewport_size = glm::vec2(1920.0f, 1080.0f);
};

#endif // CAMERA_H
