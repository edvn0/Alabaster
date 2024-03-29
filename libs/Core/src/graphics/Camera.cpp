#include "av_pch.hpp"

#include "graphics/Camera.hpp"

#include "core/Application.hpp"
#include "core/Input.hpp"

namespace Alabaster {

	Camera::Camera(const glm::mat4& projection, const glm::mat4& unreversed_projection)
		: projection_matrix(projection)
		, unreversed_projection_matrix(unreversed_projection) {};

	Camera::Camera(const float degree_fov, const float width, const float height, const float near_plane, const float far_plane)
		: projection_matrix(glm::perspectiveFov(glm::radians(degree_fov), width, height, far_plane, near_plane))
		, unreversed_projection_matrix(glm::perspectiveFov(glm::radians(degree_fov), width, height, near_plane, far_plane)) {};

	void Camera::set_ortho_projection_matrix(const float width, const float height, const float near_plane, const float far_plane)
	{
		projection_matrix = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, far_plane, near_plane);
		unreversed_projection_matrix = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, near_plane, far_plane);
	}

	void Camera::set_perspective_projection_matrix(
		const float radians_fov, const float width, const float height, const float near_plane, const float far_plane)
	{
		projection_matrix = glm::perspectiveFov(radians_fov, width, height, far_plane, near_plane);
		unreversed_projection_matrix = glm::perspectiveFov(radians_fov, width, height, near_plane, far_plane);
	}

	EditorCamera::EditorCamera(
		const float degree_fov, const float width, const float height, const float near_plane, const float far_plane, EditorCamera* previous_camera)
		: Camera(glm::perspectiveFov(glm::radians(degree_fov), width, height, near_plane, far_plane),
			glm::perspectiveFov(glm::radians(degree_fov), width, height, far_plane, near_plane))
		, focal_point(0.0f)
		, vertical_fov(glm::radians(degree_fov))
		, near_clip(near_plane)
		, far_clip(far_plane)
	{
		if (previous_camera) {
			position = previous_camera->position;
			position_delta = previous_camera->position_delta;
			yaw = previous_camera->yaw;
			yaw_delta = previous_camera->yaw_delta;
			pitch = previous_camera->pitch;
			pitch_delta = previous_camera->pitch_delta;

			focal_point = previous_camera->focal_point;
		}

		distance = glm::distance(position, focal_point);

		position = calculate_position();
		const glm::quat orientation = get_orientation();
		direction = glm::eulerAngles(orientation) * (180.0f / glm::pi<float>());
		view_matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4(orientation);
		view_matrix = glm::inverse(view_matrix);
	}

	void EditorCamera::init(EditorCamera* previous_camera)
	{
		if (previous_camera) {
			position = previous_camera->position;
			position_delta = previous_camera->position_delta;
			yaw = previous_camera->yaw;
			yaw_delta = previous_camera->yaw_delta;
			pitch = previous_camera->pitch;
			pitch_delta = previous_camera->pitch_delta;

			focal_point = previous_camera->focal_point;
		}

		distance = glm::distance(position, focal_point);

		position = calculate_position();
		const glm::quat orientation = get_orientation();
		direction = glm::eulerAngles(orientation) * (180.0f / glm::pi<float>());
		view_matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4(orientation);
		view_matrix = glm::inverse(view_matrix);
	}

	void EditorCamera::on_update(const float ts)
	{
		const glm::vec2& mouse = Input::mouse_position();
		const glm::vec2 delta = (mouse - initial_mouse_position) * 0.002f;

		if (Input::mouse(Mouse::Right) && !Input::key(Key::LeftControl)) {
			camera_mode = CameraMode::Flycam;
			const float yaw_sign = get_up_direction().y < 0 ? -1.0f : 1.0f;

			const float speed = get_camera_speed();

			if (Input::key(Key::Q)) {
				position_delta -= ts * speed * glm::vec3 { 0.f, yaw_sign, 0.f };
			}
			if (Input::key(Key::E)) {
				position_delta += ts * speed * glm::vec3 { 0.f, yaw_sign, 0.f };
			}
			if (Input::key(Key::S)) {
				position_delta -= ts * speed * direction;
			}
			if (Input::key(Key::W)) {
				position_delta += ts * speed * direction;
			}
			if (Input::key(Key::A)) {
				position_delta -= ts * speed * right_direction;
			}
			if (Input::key(Key::D)) {
				position_delta += ts * speed * right_direction;
			}

			constexpr float max_rate { 0.12f };
			yaw_delta += glm::clamp(yaw_sign * delta.x * rotation_speed(), -max_rate, max_rate);
			pitch_delta += glm::clamp(delta.y * rotation_speed(), -max_rate, max_rate);

			right_direction = glm::cross(direction, glm::vec3 { 0.f, yaw_sign, 0.f });

			const auto angle_axis_right = glm::angleAxis(-yaw_delta, glm::vec3 { 0.f, yaw_sign, 0.f });
			const auto angle_axis_left = glm::angleAxis(-pitch_delta, right_direction);
			const auto cross_product = glm::cross(angle_axis_left, angle_axis_right);
			const auto normalised = glm::normalize(cross_product);
			direction = glm::rotate(normalised, direction);

			const float actual_distance = glm::distance(focal_point, position);
			focal_point = position + get_forward_direction() * actual_distance;
			distance = actual_distance;
		} else if (Input::key(Key::LeftControl)) {
			camera_mode = CameraMode::Arcball;

			if (Input::mouse(Mouse::Middle)) {
				mouse_pan(delta);
			} else if (Input::mouse(Mouse::Left)) {
				mouse_rotate(delta);
			} else if (Input::mouse(Mouse::Right)) {
				mouse_zoom(delta.x + delta.y);
			}
		}

		initial_mouse_position = mouse;
		position += position_delta;
		yaw += yaw_delta;
		pitch += pitch_delta;

		if (camera_mode == CameraMode::Arcball)
			position = calculate_position();

		update_camera_view();
	}

	float EditorCamera::get_camera_speed() const
	{
		float speed = normal_speed;
		if (Input::key(Key::LeftControl))
			speed /= 2 - glm::log(normal_speed);
		if (Input::key(Key::LeftShift))
			speed *= 2 - glm::log(normal_speed);

		return glm::clamp(speed, min_speed, max_speed);
	}

	void EditorCamera::update_camera_view()
	{
		const float yaw_sign = get_up_direction().y < 0 ? -1.0f : 1.0f;

		const float cos_angle = glm::dot(get_forward_direction(), get_up_direction());
		if (cos_angle * yaw_sign > 0.99f)
			pitch_delta = 0.f;

		const glm::vec3 look_at = position + get_forward_direction();
		direction = glm::normalize(look_at - position);
		distance = glm::distance(position, focal_point);
		view_matrix = glm::lookAt(position, look_at, glm::vec3 { 0.f, yaw_sign, 0.f });

		yaw_delta *= 0.6f;
		pitch_delta *= 0.6f;
		position_delta *= 0.8f;
	}

	void EditorCamera::focus(const glm::vec3& focus_point)
	{
		focal_point = focus_point;
		camera_mode = CameraMode::Flycam;
		if (distance > min_focus_distance) {
			distance -= distance - min_focus_distance;
			position = focal_point - get_forward_direction() * distance;
		}
		position = focal_point - get_forward_direction() * distance;
		update_camera_view();
	}

	std::pair<float, float> EditorCamera::pan_speed() const
	{
		const float x = glm::min(float(viewport_width) / 1000.0f, 2.4f); // max = 2.4f
		const float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = glm::min(float(viewport_height) / 1000.0f, 2.4f); // max = 2.4f
		const float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { x_factor, y_factor };
	}

	float EditorCamera::rotation_speed() const { return 0.3f; }

	float EditorCamera::zoom_speed() const
	{
		float dist = distance * 0.2f;
		dist = glm::max(dist, 0.0f);
		float speed = dist * dist;
		speed = glm::min(speed, 50.0f); // max speed = 50
		return speed;
	}

	void EditorCamera::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return on_mouse_scroll(e); });
	}

	bool EditorCamera::on_mouse_scroll(MouseScrolledEvent& e)
	{
		if (Input::mouse(Mouse::Right)) {
			normal_speed += e.get_y_offset() * 0.3f * normal_speed;
			normal_speed = std::clamp(normal_speed, min_speed, max_speed);
		} else {
			mouse_zoom(e.get_y_offset() * 0.1f);
			update_camera_view();
		}

		return true;
	}

	void EditorCamera::mouse_pan(const glm::vec2& delta)
	{
		auto&& [x_velocity, y_velocity] = pan_speed();
		focal_point -= get_right_direction() * delta.x * x_velocity * distance;
		focal_point += get_up_direction() * delta.y * y_velocity * distance;
	}

	void EditorCamera::mouse_rotate(const glm::vec2& delta)
	{
		const float yaw_sign = get_up_direction().y < 0.0f ? -1.0f : 1.0f;
		yaw_delta += yaw_sign * delta.x * rotation_speed();
		pitch_delta += delta.y * rotation_speed();
	}

	void EditorCamera::mouse_zoom(float delta)
	{
		distance -= delta * zoom_speed();
		const glm::vec3 forward_dir = get_forward_direction();
		position = focal_point - forward_dir * distance;
		if (distance < 1.0f) {
			focal_point += forward_dir * distance;
			distance = 1.0f;
		}
		position_delta += delta * zoom_speed() * forward_dir;
	}

	glm::vec3 EditorCamera::get_up_direction() const { return glm::rotate(get_orientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }

	glm::vec3 EditorCamera::get_right_direction() const { return glm::rotate(get_orientation(), glm::vec3(1.f, 0.f, 0.f)); }

	glm::vec3 EditorCamera::get_forward_direction() const { return glm::rotate(get_orientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

	glm::vec3 EditorCamera::calculate_position() const { return focal_point - get_forward_direction() * distance + position_delta; }

	glm::quat EditorCamera::get_orientation() const { return glm::quat(glm::vec3(-pitch - pitch_delta, -yaw - yaw_delta, 0.0f)); }

} // namespace Alabaster
