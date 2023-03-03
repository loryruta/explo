#include "DeviceBuffer.hpp"

#include "Renderer.hpp"

using namespace explo;

DeviceBuffer::DeviceBuffer(
	Renderer& renderer,
	VkMemoryPropertyFlags memory_properties,
	VkBufferUsageFlags buffer_usages,
	size_t init_size
	) :
	m_renderer(renderer),
	m_memory_properties(memory_properties),
	m_buffer_usages(buffer_usages),
	m_size(init_size)
{
	m_buffer = std::make_shared<vren::vk_utils::buffer>(create_buffer(init_size));
}

DeviceBuffer::~DeviceBuffer()
{
}

void DeviceBuffer::write(void* data, size_t data_size, size_t offset)
{
	// TODO IMPROVEMENT: Cache and re-use the staging buffers; avoid allocating each write

	std::shared_ptr<vren::vk_utils::buffer> staging_buffer = std::make_shared<vren::vk_utils::buffer>(
		vren::vk_utils::alloc_host_visible_buffer(
			m_renderer.m_context,
			m_buffer_usages,
			data_size,
			true // persistently_mapped
			)
		);

	vren::vk_utils::immediate_transfer_queue_submit(m_renderer.m_context,
		[&](VkCommandBuffer command_buffer, vren::resource_container& resource_container)
		{
			vkCmdUpdateBuffer(
				command_buffer,
				staging_buffer->m_buffer.m_handle,
				offset,
				data_size,
				data
				);

			// Add to resource_container isn't necessary because the command executes immediately
			//resource_container.add_resource(staging_buffer);
		});

	Op op{};
	op.m_type = OP_WRITE;

	WriteOp& write_op = op.m_write;
	write_op.m_staging_buffer = staging_buffer;
	write_op.m_src_offset = offset;
	write_op.m_size = data_size;
	write_op.m_dst_offset = 0;

	m_operations.push_back(op);
}

void DeviceBuffer::resize(size_t size)
{
	if (size <= m_size) // If shrinking, then we don't need to allocate a new buffer
	{
		m_size = size;
		return;
	}

	Op op{};
	op.m_type = OP_RESIZE;

	ResizeOp& resize_op = op.m_resize;
	resize_op.m_size = size;

	m_operations.push_back(op);
}

void DeviceBuffer::record(VkCommandBuffer command_buffer, vren::resource_container& resource_container)
{
	// TODO IMPROVEMENT: this could be replaced with a compute shader that performs all the copies in parallel

	for (Op const& op : m_operations)
	{
		switch (op.m_type)
		{
		case OP_WRITE:
			perform_write(command_buffer, resource_container, op.m_write);
			break;
		case OP_RESIZE:
			perform_resize(command_buffer, resource_container, op.m_resize);
			break;
		default:
			throw std::runtime_error("Invalid operation type");
		}
	}
	m_operations.clear();
}

void DeviceBuffer::perform_write(
	VkCommandBuffer command_buffer,
	vren::resource_container& resource_container,
	WriteOp const& write_op
)
{
	VkBufferCopy buffer_copy{};
	buffer_copy.srcOffset = write_op.m_src_offset;
	buffer_copy.dstOffset = write_op.m_dst_offset;
	buffer_copy.size = write_op.m_size;

	vkCmdCopyBuffer(
		command_buffer,
		write_op.m_staging_buffer->m_buffer.m_handle,
		m_buffer->m_buffer.m_handle,
		1,
		&buffer_copy
		);

	resource_container.add_resources(write_op.m_staging_buffer, m_buffer);
}

void DeviceBuffer::perform_resize(
	VkCommandBuffer command_buffer,
	vren::resource_container& resource_container,
	ResizeOp const& resize_op
	)
{
	double scale_factor = glm::ceil(double(m_size) / double(4096)); // TODO minimize resizes
	size_t new_size = scale_factor * 4096;

	std::shared_ptr<vren::vk_utils::buffer> new_buffer =
		std::make_shared<vren::vk_utils::buffer>(create_buffer(new_size));

	VkBufferCopy buffer_copy{};
	buffer_copy.srcOffset = 0;
	buffer_copy.dstOffset = 0;
	buffer_copy.size = m_size;

	vkCmdCopyBuffer(
		command_buffer,
		m_buffer->m_buffer.m_handle,
		new_buffer->m_buffer.m_handle,
		1,
		&buffer_copy
		);

	resource_container.add_resources(m_buffer, new_buffer);

	m_buffer = new_buffer;
	m_size = resize_op.m_size;
}

vren::vk_utils::buffer DeviceBuffer::create_buffer(size_t size)
{
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = m_buffer_usages | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.requiredFlags = m_memory_properties;

	VkBuffer buffer;
	VmaAllocation alloc;
	VmaAllocationInfo alloc_info;
	vmaCreateBuffer(m_renderer.m_context.m_vma_allocator, &buffer_info, &alloc_create_info, &buffer, &alloc, &alloc_info);

	return vren::vk_utils::buffer{
		.m_buffer = vren::vk_buffer(m_renderer.m_context, buffer),
		.m_allocation = vren::vma_allocation (m_renderer.m_context, alloc),
		.m_allocation_info = alloc_info,
	};
}
