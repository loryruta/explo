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
		struct WriteOp
		{
			std::shared_ptr<vren::vk_utils::buffer> m_staging_buffer;
			uint64_t m_src_offset;
			uint64_t m_size;
			uint64_t m_dst_offset;
		};

		struct ResizeOp
		{
			uint64_t m_size;
		};

		enum OpType : uint32_t { OP_WRITE, OP_RESIZE };

		struct Op
		{
			OpType m_type;

			WriteOp m_write;   // OP_WRITE
			ResizeOp m_resize; // OP_RESIZE
		};

	private:
		Renderer& m_renderer;

		VkMemoryPropertyFlags m_memory_properties;
		VkBufferUsageFlags m_buffer_usages;

		std::shared_ptr<vren::vk_utils::buffer> m_buffer;
		size_t m_size = 0;

		std::vector<Op> m_operations;

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

		void write(void* data, size_t data_size, size_t offset);
		void resize(size_t init_size);

		void record(VkCommandBuffer command_buffer, vren::resource_container& resource_container);

	private:
		vren::vk_utils::buffer create_buffer(size_t size);

		void perform_write(
			VkCommandBuffer command_buffer,
			vren::resource_container& resource_container,
			WriteOp const& write_op
			);

		void perform_resize(
			VkCommandBuffer command_buffer,
			vren::resource_container& resource_container,
			ResizeOp const& resize_op
			);
	};

} // namespace explo
