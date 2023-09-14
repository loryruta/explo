#pragma once

#include <cstdint>
#include <vector>

#include "vk_mem_alloc.h"

namespace explo
{
    /// An allocator that allows resizing of the managed memory.
    class VirtualAllocator
    {
    private:
        size_t m_size;
        size_t m_alignment;

        size_t m_page_size;

        // If the i-th entry isn't null, the i-th page is reserved. If the MSB is set it means the allocation also takes the
        // next page
        std::vector<uint8_t> m_page_status;

        int m_num_allocated_pages = 0;

    public:
        /// \param size The initial size of the allocable space.
        /// \param alignment Every allocation offset will be a multiple of this number (0 means not set).
        /// \param min_page_size The actual page size will be at least this value and a multiple of alignment.
        explicit VirtualAllocator(size_t size, size_t alignment, size_t min_page_size);
        ~VirtualAllocator();

        size_t get_size() const { return m_size; }
        int get_num_allocated_pages() const { return m_num_allocated_pages; }

        /// Resizes the managed memory to the given one.
        void resize(size_t new_size);

        /// Allocates a block memory of the requested size and returns its offset.
        /// Returns false if the allocation was unsuccessful. In this case offset has an undefined value.
        bool allocate(size_t size, size_t &offset);

        /// Frees the block of memory corresponding to the given offset.
        /// Does nothing if the block wasn't allocated.
        void free(size_t offset);
    };
}  // namespace explo