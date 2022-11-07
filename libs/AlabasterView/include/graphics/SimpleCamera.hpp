#pragma once

#include "core/events/Event.hpp"

#include <glm/glm.hpp>

namespace Alabaster {

	enum class CameraType { LookAt, FirstPerson };

	class SimpleCamera {
	public:
		SimpleCamera(CameraType type, float aspect, float near, float far, float fov);

		void on_event(Event& event);
		void on_update(float ts);

		const glm::mat4& get_projection_matrix() const;
		const glm::mat4& get_view_matrix() const;
		float get_near_clip() const;
		float get_far_clip() const;

		void set_perspective(float field_of_view_degrees, float aspect_ratio, float in_near_plane, float in_far_plane);

		void update_aspect_ratio(float aspect_ratio);

		void set_position(glm::vec3 pos);
		void set_rotation(glm::vec3 rot);
		void rotate(glm::vec3 delta);
		void set_translation(glm::vec3 translate);
		void translate(glm::vec3 delta);
		void set_rotation_speed(float speed);
		void set_movement_speed(float speed);

	private:
		void update_view_matrix();

		CameraType type = CameraType::LookAt;

		float rotation_speed = 1.0f;
		float movement_speed = 1.0f;

		bool updated = false;
		bool should_flip_y_axis = false;

		struct {
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;

		float field_of_view;
		float near_plane;
		float far_plane;
		float aspect { 1.666 };

		glm::vec3 rotation {};
		glm::vec3 position {};
		glm::vec4 view_position {};
	};
} // namespace Alabaster
