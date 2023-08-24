#include "VirtualAllocator.hpp"

#include <cassert>
#include <cmath>

using namespace explo;

VirtualAllocator::VirtualAllocator(size_t size)
{
	resize(size);
}

VirtualAllocator::~VirtualAllocator()
{
}

void VirtualAllocator::resize(size_t new_size)
{
	m_size = new_size;
	m_page_status.resize(new_size);
}

bool VirtualAllocator::allocate(size_t size, size_t& offset)
{
	int num_required_pages = ceil(double(size) / double(k_page_size));

	int found_page_start_idx = -1;
	int num_found_pages = 0;

	for (int i = 0; i < m_page_status.size(); i++)
	{
		if (!m_page_status.at(i))
		{
			if (found_page_start_idx < 0)
				found_page_start_idx = i;

			num_found_pages++;
			if (num_found_pages == num_required_pages)
				break;
		}
		else
		{
			found_page_start_idx = -1;
			num_found_pages = 0;
		}
	}

	if (found_page_start_idx >= 0)
	{
		offset = found_page_start_idx;
		return true;
	}

	return false;
}

void VirtualAllocator::free(size_t offset)
{
	int start_page_idx = floor(double(offset) / double(k_page_size));
	assert(m_page_status.at(start_page_idx));

	m_page_status[start_page_idx] = false;

	for (int i = start_page_idx; i < m_page_status.size(); i++)
	{
		if (!m_page_status.at(i))
			break;

		m_page_status[i] = false;
	}
}
