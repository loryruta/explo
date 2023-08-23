#include "BlockRegistry.hpp"

#include "video/RenderApi.hpp"

using namespace explo;

BlockRegistry::BlockRegistry()
{
	m_block_data.push_back({ .m_color = 0x00000000 }); // Air
	m_block_data.push_back({ .m_color = 0xff00ff00 }); // Grass
	m_block_data.push_back({ .m_color = 0xffff00ff }); // Dirt
	m_block_data.push_back({ .m_color = 0xeeeeeeff }); // Stone
	m_block_data.push_back({ .m_color = 0xffffffff }); // Snow

	RenderApi::block_registry_upload(*this);
}
