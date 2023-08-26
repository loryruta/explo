#include "VirtualAllocator.hpp"

#include <cassert>
#include <cmath>

using namespace explo;

VirtualAllocator::VirtualAllocator(size_t size, size_t alignment, size_t min_page_size) :
	m_alignment(alignment)
{
	// Calc page size (ensure it's a multiple of alignment)
	if (m_alignment > 0)
	{
		m_page_size = m_alignment;
		while (m_page_size < min_page_size)
			m_page_size *= 2;
	}
	else m_page_size = min_page_size;

	resize(size);
}

VirtualAllocator::~VirtualAllocator()
{
}

void VirtualAllocator::resize(size_t new_size)
{
	size_t num_pages = std::ceil(double(new_size) / double(m_page_size));
	new_size = num_pages * m_page_size;

	m_size = new_size;
	m_page_status.resize(num_pages);
}

bool VirtualAllocator::allocate(size_t size, size_t& offset)
{
	int num_required_pages = std::ceil(double(size) / double(m_page_size));

	int found_page_start_idx = -1;
	int num_found_pages = 0;

	for (int i = 0; i < m_page_status.size(); i++)
	{
		if (m_page_status.at(i) == 0)
		{
			if (found_page_start_idx < 0) found_page_start_idx = i;

			num_found_pages++;
			if (num_found_pages == num_required_pages) break;
		}
		else
		{
			found_page_start_idx = -1;
			num_found_pages = 0;
		}
	}

	if (found_page_start_idx >= 0)
	{
		offset = found_page_start_idx * m_page_size;

		for (int i = 0; i < num_required_pages - 1; i++)
			m_page_status[found_page_start_idx + i] = 0xFF;
		m_page_status[found_page_start_idx + num_required_pages - 1] = 0x7F; // MSB not set because there's no continuation page

		m_num_allocated_pages += num_required_pages;

		assert(m_alignment > 0 && offset % m_alignment == 0);

		return true;
	}
	else return false;
}

void VirtualAllocator::free(size_t offset)
{
	int start_page_idx = std::floor(double(offset) / double(m_page_size));
	assert((m_page_status.at(start_page_idx) & 0x7F) != 0);
	assert(start_page_idx == 0 || (m_page_status.at(start_page_idx - 1) & 0x80) == 0); // The previous page shouldn't be a continuation page

	for (int i = start_page_idx; i < m_page_status.size(); i++)
	{
		uint8_t page_status = m_page_status.at(i);
		assert((page_status & 0x7F) != 0);

		m_page_status[i] = 0;
		m_num_allocated_pages--;

		if ((page_status & 0x80) == 0) break;
	}
}
