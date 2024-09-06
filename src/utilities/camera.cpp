#include "camera.h"
#include "main.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

float camera::get_target_distance()
{
	return glm::length(camera_position - camera_target);
}

void camera::set_target_distance(float distance)
{
	glm::vec3 temp_position = distance * (glm::normalize(camera_position - camera_target));

	camera_position = camera_target + temp_position;
}

void camera::translate_target(glm::vec3 delta)
{

	glm::mat4 view_matrix = glm::lookAt(camera_position,
										camera_target,
										camera_up_vector);

	glm::mat4 projection_matrix = glm::perspective( field_of_view,
													aspect_ratio,
													near_clip,
													far_clip);

	// Scale the delta by the distance to the target: this way, when zoomed in, you won't go too fast and when zoomed out you won't go too slow.
	glm::vec4 delta_vector = glm::vec4(glm::length(camera_position - camera_target) * delta, 0.0);

	glm::vec4 temp_target = glm::vec4(camera_target, 1.0) + glm::inverse(view_matrix) * delta_vector;
	glm::vec4 temp_position = glm::vec4(camera_position, 1.0) + glm::inverse(view_matrix) * delta_vector;

	camera_target = glm::vec3(temp_target.x, temp_target.y, temp_target.z) / temp_target.w;
	camera_position = glm::vec3(temp_position.x, temp_position.y, temp_position.z) / temp_position.w;

	//camera_target = camera_target + delta;
	//camera_position = camera_position + delta;
}

void camera::rotate_around_target(glm::vec2 rotation)
{
	glm::vec3 temp_position = camera_position - camera_target;

	// Calculate yaxis:
	glm::vec3 zaxis = glm::normalize(camera_target - camera_position);
	glm::vec3 xaxis = glm::normalize(glm::cross(camera_up_vector, zaxis));
	glm::vec3 yaxis = glm::cross(zaxis, xaxis);

	if (rotation_pivot_type == free_move)
		camera_up_vector = yaxis;

	temp_position = glm::rotate(temp_position, rotation.y, glm::cross(camera_up_vector, temp_position));
	temp_position = glm::rotate(temp_position, rotation.x, flip_x_axis*camera_up_vector);

	camera_position = temp_position + camera_target;

	if (rotation_pivot_type == free_around_axis)
	{
		// Prevent gimbal lock:
		glm::vec3 zaxis2 = glm::normalize(camera_target - camera_position);
		glm::vec3 xaxis2 = glm::normalize(glm::cross(camera_up_vector, zaxis2));
		glm::vec3 yaxis2 = glm::cross(zaxis2, xaxis2);


		if (glm::dot(yaxis, yaxis2) < 0.0)
		{
			camera_up_vector = -camera_up_vector;
			flip_x_axis = -flip_x_axis;
		}
	}

}

glm::mat4 camera::get_view_matrix()
{
	glm::mat4 view_matrix = glm::lookAt(camera_position,
										camera_target,
										camera_up_vector);

	return (view_matrix);
}

glm::mat4 camera::get_projection_matrix()
{
	glm::mat4 projection_matrix = glm::perspective( field_of_view,
													aspect_ratio,
													near_clip,
													far_clip);

	return (projection_matrix);
}

glm::mat4 camera::get_view_projection_matrix()
{
	glm::mat4 view_matrix = glm::lookAt(camera_position,
										camera_target,
										camera_up_vector);

	glm::mat4 projection_matrix = glm::perspective( field_of_view,
													aspect_ratio,
													near_clip,
													far_clip);

	return (projection_matrix * view_matrix);
}
