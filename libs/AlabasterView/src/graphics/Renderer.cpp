#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

#include "core/Common.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/SceneRenderer.hpp"
#include "utilities/FileInputOutput.hpp"

namespace Alabaster {

	static std::unordered_map<std::string, Shader> shaders;

	static bool renderer_is_initialized { false };
	static bool scene_renderer_is_initialized { false };
	static bool frame_started { false };

	static SceneRenderer* scene_renderer { nullptr };

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

	void Renderer::basic_mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Camera>& camera, const std::unique_ptr<Pipeline>& pipeline)
	{
		api().basic_mesh(mesh, camera, pipeline);
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
		if (!renderer_is_initialized) {
			renderer_is_initialized = true;
			api().init();
		}

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

	SceneRenderer& Renderer::api()
	{
		if (!scene_renderer_is_initialized) {
			scene_renderer = new SceneRenderer();
			scene_renderer_is_initialized = true;
		}
		return *scene_renderer;
	}

} // namespace Alabaster
