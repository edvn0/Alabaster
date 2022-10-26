#pragma once

typedef struct VkRenderPass_T* VkRenderPass;

namespace Alabaster {

	struct RenderPassSpecification { };

	class RenderPass {
	public:
		explicit RenderPass(const RenderPassSpecification& spec)
			: spec(spec)
		{
			create_render_pass();
		};

		VkRenderPass get_render_pass();

	private:
		void create_render_pass();

		RenderPassSpecification spec;
		VkRenderPass render_pass;
	};

} // namespace Alabaster
