#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

#include "core/Common.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Pipeline.hpp"
#include "utilities/FileInputOutput.hpp"

namespace Alabaster {

	static std::unordered_map<std::string, Shader> shaders;

	static bool renderer_is_initialized { false };
	static bool scene_renderer_is_initialized { false };
	static bool frame_started { false };

	RenderQueue& Renderer::render_queue()
	{
		static RenderQueue render_queue;
		return render_queue;
	}

	void Renderer::execute()
	{
		verify(renderer_is_initialized, "Renderer should be initialized.");
		render_queue().execute();
	}

	void Renderer::begin()
	{
		verify(!frame_started);
		frame_started = true;
		Log::info("[Renderer] Begin frame.");
	}

	void Renderer::end()
	{
		verify(frame_started);
		frame_started = false;
		Log::info("[Renderer] End frame.");
	}

	void Renderer::init()
	{
		Log::info("[Renderer] Initialisation of renderer.");

		renderer_is_initialized = true;

		const auto all_files_in_shaders = IO::in_directory("app/resources/shaders", { ".spv" });

		for (const auto& shader : all_files_in_shaders) {
			Log::info("[Renderer] Shader found: {}", shader.filename());
		}
	}

	void Renderer::shutdown()
	{
		Log::info("[Renderer] Destruction of renderer.");
		// Destruction code.
	}

} // namespace Alabaster
