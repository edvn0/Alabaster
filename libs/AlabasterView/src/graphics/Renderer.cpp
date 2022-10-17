#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

namespace Alabaster {

	RenderQueue& Renderer::render_queue()
	{
		static RenderQueue render_queue;
		return render_queue;
	}

} // namespace Alabaster
