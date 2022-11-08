#include "av_pch.hpp"

#include "graphics/SimpleCamera.hpp"

#include "codes/KeyCode.hpp"
#include "core/Application.hpp"
#include "core/events/Event.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Alabaster {

	SimpleCamera::SimpleCamera(CameraType type, float aspect, float near, float far, float fov)
		: type(type)
		, aspect(aspect)
		, near_plane(near)
		, far_plane(far)
		, field_of_view(fov)
	{
		set_perspective(fov, aspect, near, far);
		update_view_matrix();
	};

	void SimpleCamera::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>([this, &type = type, &flip = should_flip_y_axis](KeyPressedEvent& in) {
			if (in.get_key_code() == Key::V) {
				flip = !flip;
			}
			if (in.get_key_code() == Key::B) {
				type = type == CameraType::FirstPerson ? CameraType::LookAt : CameraType::FirstPerson;
			}

			if (in.get_key_code() == Key::I) {
				rotate({ -1, 0, 0 });
			}

			if (in.get_key_code() == Key::K) {
				rotate({ 1, 0, 0 });
			}

			if (in.get_key_code() == Key::J) {
				rotate({ 0, -1, 0 });
			}

			if (in.get_key_code() == Key::L) {
				rotate({ 0, 1, 0 });
			}

			if (in.get_key_code() == Key::R) {
				set_position({ 0, -5, -5 });
				set_rotation({ 0, 0, 0 });
			}

			return false;
		});
	}

	void SimpleCamera::on_update(float ts)
	{
		const auto maybe_new_aspect = Application::the().swapchain().aspect_ratio();
		if (std::fabs(aspect - maybe_new_aspect) >= 1e-5) {
			update_aspect_ratio(maybe_new_aspect);
			Log::info("[SimpleCamera] New aspect ratio: {}", aspect);
		}

		updated = false;
		if (type == CameraType::FirstPerson) {
			const auto any_movement_key_pressed = Input::any(Key::A, Key::W, Key::S, Key::D);

			if (any_movement_key_pressed) {
				glm::vec3 cam_front;
				cam_front.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
				cam_front.y = sin(glm::radians(rotation.x));
				cam_front.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
				cam_front = glm::normalize(cam_front);

				float move_speed = ts * movement_speed;

				if (Input::key(Key::W))
					position += cam_front * move_speed;
				if (Input::key(Key::S))
					position -= cam_front * move_speed;
				if (Input::key(Key::A))
					position -= glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
				if (Input::key(Key::D))
					position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;

				update_view_matrix();
			}
		}
	}

	const glm::mat4& SimpleCamera::get_projection_matrix() const { return matrices.perspective; }

	const glm::mat4& SimpleCamera::get_view_matrix() const { return matrices.view; }

	float SimpleCamera::get_near_clip() const { return near_plane; }

	float SimpleCamera::get_far_clip() const { return far_plane; }

	void SimpleCamera::set_perspective(float field_of_view_degrees, float aspect_ratio, float in_near_plane, float in_far_plane)
	{
		field_of_view = glm::radians(field_of_view_degrees);
		near_plane = in_near_plane;
		far_plane = in_far_plane;
		aspect = aspect_ratio;
		matrices.perspective = glm::perspective(field_of_view, aspect, near_plane, far_plane);
	};

	void SimpleCamera::update_aspect_ratio(float aspect_ratio)
	{
		aspect = aspect_ratio;
		matrices.perspective = glm::perspective(field_of_view, aspect, near_plane, far_plane);
	}

	void SimpleCamera::set_position(glm::vec3 pos)
	{
		position = pos;
		update_view_matrix();
	}

	void SimpleCamera::set_rotation(glm::vec3 rot)
	{
		rotation = rot;
		update_view_matrix();
	}
	void SimpleCamera::rotate(glm::vec3 delta)
	{
		rotation += delta;
		update_view_matrix();
	}

	void SimpleCamera::set_translation(glm::vec3 translate)
	{
		position = translate;
		update_view_matrix();
	};
	void SimpleCamera::translate(glm::vec3 delta)
	{
		position += delta;
		update_view_matrix();
	}

	void SimpleCamera::set_rotation_speed(float speed) { rotation_speed = speed; }
	void SimpleCamera::set_movement_speed(float speed) { movement_speed = speed; }

	void SimpleCamera::update_view_matrix()
	{
		glm::mat4 rotation_matrix = glm::mat4(1.0f);
		glm::mat4 translation_matrix { 1.0f };

		rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = position;
		translation_matrix = glm::translate(glm::mat4(1.0f), translation);

		if (type == CameraType::FirstPerson) {
			matrices.view = rotation_matrix * translation_matrix;
		} else {
			matrices.view = translation_matrix * rotation_matrix;
		}

		view_position = glm::vec4(position, 0.0f);
		;

		updated = true;
	};
}; // namespace Alabaster
