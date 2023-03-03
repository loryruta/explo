#pragma once

#include <cstdint>
#include <vector>

#include "vk_mem_alloc.h"

namespace explo
{
	/// An allocator that allows resizing of the managed memory.
	class VirtualAllocator
	{
	public:
		static constexpr size_t k_page_size = 4096;

	private:
		size_t m_size;

		std::vector<bool> m_page_status;

	public:
		explicit VirtualAllocator(size_t size);
		~VirtualAllocator();

		size_t get_size() const { return m_size; }

		/// Resizes the managed memory to the given one.
		void resize(size_t new_size);

		/// Allocates a block memory of the requested size and returns its offset.
		/// Returns false if the allocation was unsuccessful. In this case offset has an undefined value.
		bool allocate(size_t size, size_t& offset);

		/// Frees the block of memory corresponding to the given offset.
		/// Does nothing if the block wasn't allocated.
		void free(size_t offset);
	};
} // namespace explo