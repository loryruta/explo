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
			m_buffer_usages | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			data_size,
			false // Persistently mapped
			)
		);

	vren::vk_utils::set_name(m_renderer.m_context, *staging_buffer, "DeviceBuffer-Write-StagingBuffer");

	void* staging_buffer_data;
	vmaMapMemory(m_renderer.m_context.m_vma_allocator, staging_buffer->m_allocation.m_handle, &staging_buffer_data);
	{
		std::memcpy(staging_buffer_data, data, data_size);
	}
	vmaUnmapMemory(m_renderer.m_context.m_vma_allocator, staging_buffer->m_allocation.m_handle);

	// Enqueue a copy operation to copy data from the staging buffer to the actual device buffer
	// TODO this is the same as registering the copy on a secondary command buffer; we have to review vren's command_pool/command_buffer API first
	CopyOp copy_op{};
	copy_op.m_src_buffer = staging_buffer;
	copy_op.m_dst_buffer = m_buffer;
	copy_op.m_src_offset = 0;
	copy_op.m_dst_offset = offset;
	copy_op.m_size = data_size;

	m_operations.push_back(copy_op);
}

void DeviceBuffer::resize(size_t size)
{
	if (size <= m_size) return;

	const size_t k_grow_factor = 4096; // TODO Change this in order to minimize resizes
	size_t new_size = glm::ceil(double(size) / double(k_grow_factor)) * k_grow_factor;

	std::shared_ptr<vren::vk_utils::buffer> new_buffer =
		std::make_shared<vren::vk_utils::buffer>(create_buffer(new_size));

	// Enqueue a copy operation to copy data from the old buffer to the new buffer
	CopyOp copy_op{};
	copy_op.m_src_buffer = m_buffer;
	copy_op.m_dst_buffer = new_buffer;
	copy_op.m_src_offset = 0;
	copy_op.m_dst_offset = 0;
	copy_op.m_size = std::min(m_size, new_size);

	m_operations.push_back(copy_op);

	// Update the state
	m_buffer = new_buffer;
	m_size = new_size;
}

void DeviceBuffer::record(VkCommandBuffer command_buffer, vren::resource_container& resource_container)
{
	// TODO IMPROVEMENT: this could be replaced with a compute shader that performs all the copies in parallel
	// TODO we only have OP_WRITE, don't make an enum and simplify Op

	for (CopyOp const& copy_op : m_operations) perform_copy(command_buffer, resource_container, copy_op);

	m_operations.clear();
}

void DeviceBuffer::perform_copy(
	VkCommandBuffer command_buffer,
	vren::resource_container& resource_container,
	CopyOp const& copy_op
)
{
	VkBufferCopy buffer_copy{};
	buffer_copy.srcOffset = copy_op.m_src_offset;
	buffer_copy.dstOffset = copy_op.m_dst_offset;
	buffer_copy.size = copy_op.m_size;

	vkCmdCopyBuffer(
		command_buffer,
		copy_op.m_src_buffer->m_buffer.m_handle, // srcBuffer
		copy_op.m_dst_buffer->m_buffer.m_handle, // dstBuffer
		1, &buffer_copy
		);

	resource_container.add_resources(
		copy_op.m_src_buffer,
		copy_op.m_dst_buffer,
		m_buffer
		);
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

	VkBuffer buffer_handle;
	VmaAllocation alloc;
	VmaAllocationInfo alloc_info;
	vmaCreateBuffer(m_renderer.m_context.m_vma_allocator, &buffer_info, &alloc_create_info, &buffer_handle, &alloc, &alloc_info);

	vren::vk_utils::buffer buffer = vren::vk_utils::buffer{
		.m_buffer = vren::vk_buffer(m_renderer.m_context, buffer_handle),
		.m_allocation = vren::vma_allocation (m_renderer.m_context, alloc),
		.m_allocation_info = alloc_info,
		};
	return std::move(buffer);
}
