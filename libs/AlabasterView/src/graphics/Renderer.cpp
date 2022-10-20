#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

#include "core/Common.hpp"

namespace Alabaster {
	static bool renderer_is_initialized { false };

	RenderQueue& Renderer::render_queue()
	{
		static RenderQueue render_queue;
		return render_queue;
	}

	void Renderer::execute() { render_queue().execute(); }

	void Renderer::begin() { verify(renderer_is_initialized, "Renderer should be initialized."); }

	void Renderer::end() { }

	void Renderer::init()
	{
		Log::info("[Renderer] Initialisation of renderer.");
		if (!renderer_is_initialized) {
			renderer_is_initialized = true;
		}

		// Initialisation code.
	}

	void Renderer::shutdown()
	{
		Log::info("[Renderer] Destruction of renderer.");
		// Destruction code.
	}

} // namespace Alabaster
