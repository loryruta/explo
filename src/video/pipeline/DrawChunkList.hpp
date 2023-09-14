#pragma once

#include <volk.h>

#include <vren/base/resource_container.hpp>
#include <vren/pipeline/basic_renderer.hpp>

namespace explo
{
    // Forward decl
    class Renderer;

    /// A shader program which takes the chunk draw list from the Renderer and renders it to a g-buffer, using vren's basic_renderer module.
    /// The draw is indirect since the draw calls (i.e. which chunk to render) are filled dynamically by CullWorldView.
    class DrawChunkList
    {
    private:
        Renderer &m_renderer;

        vren::basic_renderer m_basic_renderer;

    public:
        explicit DrawChunkList(Renderer &renderer);
        ~DrawChunkList();

        void record(VkCommandBuffer cmd_buf, vren::resource_container &resource_container);

        vren::render_graph_t create_render_graph_node(vren::render_graph_allocator &allocator);
    };
}  // namespace explo
