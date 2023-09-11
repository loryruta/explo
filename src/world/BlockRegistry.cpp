#include "BlockRegistry.hpp"

#include "video/RenderApi.hpp"

using namespace explo;

BlockRegistry::BlockRegistry()
{
	// TODO Format is ABGR (wtf?)

	// Grass (rgb): 3ac94d
	// Dirt (rgb): 966f2a
	// Stone (rgb): 8c857a

	m_block_data.push_back({ .m_color = 0x00000000 }); // 0 = Air
	m_block_data.push_back({ .m_color = 0xff4dc93a }); // 1 = Grass
	m_block_data.push_back({ .m_color = 0xff2a6f96 }); // 2 = Dirt
	m_block_data.push_back({ .m_color = 0xff7a858c }); // 3 = Stone
	m_block_data.push_back({ .m_color = 0xffffffff }); // 4 = Snow

	RenderApi::block_registry_upload(*this);
}
