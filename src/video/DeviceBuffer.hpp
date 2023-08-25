#pragma once

#include <memory>

#include <vren/vk_helpers/buffer.hpp>
#include <vren/base/resource_container.hpp>

#include "util/VirtualAllocator.hpp"

namespace explo
{
	// Forward decl
	class Renderer;

	class DeviceBuffer
	{
	public:
		struct CopyOp
		{
			std::shared_ptr<vren::vk_utils::buffer> m_src_buffer;
			std::shared_ptr<vren::vk_utils::buffer> m_dst_buffer;
			uint64_t m_src_offset;
			uint64_t m_dst_offset;
			uint64_t m_size;
		};

	private:
		Renderer& m_renderer;

		VkMemoryPropertyFlags m_memory_properties;
		VkBufferUsageFlags m_buffer_usages;

		std::shared_ptr<vren::vk_utils::buffer> m_buffer;
		size_t m_size = 0;

		std::vector<CopyOp> m_operations;

	public:
		explicit DeviceBuffer(
			Renderer& renderer,
			VkMemoryPropertyFlags memory_properties,
			VkBufferUsageFlags buffer_usages,
			size_t init_size
			);
		~DeviceBuffer();

		std::shared_ptr<vren::vk_utils::buffer> get_buffer() const { return m_buffer; }
		size_t get_size() const { return m_size; }
		size_t get_pending_operations_count() const { return m_operations.size(); }

		void write(void* data, size_t data_size, size_t offset);
		void resize(size_t init_size);

		void record(VkCommandBuffer command_buffer, vren::resource_container& resource_container);

	private:
		vren::vk_utils::buffer create_buffer(size_t size);

		void perform_copy(
			VkCommandBuffer command_buffer,
			vren::resource_container& resource_container,
			CopyOp const& copy_op
			);
	};

} // namespace explo
