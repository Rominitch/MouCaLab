#include "Dependencies.h"

#include <LibRT/include/RTBufferDescriptor.h>
#include <LibGUI/include/GUIFontAtlas.h>

#define TEXTURE_MAX_LIMITATION 8192 

void GUIFontAtlas::Release()
{
	//Restart default value
	m_szWidth			= 0;
	m_szHeight			= 0;
	m_szNbComponants	= 0;
	m_used				= 0;

	//Clear array
	m_vNodes.clear();
	
	m_Buffer.release();
}

void GUIFontAtlas::Initialize(const size_t szWidth, const size_t szHeight, const size_t szNbComponants)
{
	MouCa::assertion(0 < szWidth  && szWidth <= TEXTURE_MAX_LIMITATION);	//DEV Issue: Limitation for current texture size
	MouCa::assertion(0 < szHeight && szHeight <= TEXTURE_MAX_LIMITATION);	//DEV Issue: Limitation for current texture size
	MouCa::assertion(0 < szNbComponants && szNbComponants <= 4);			//DEV Issue: Limitation 
	
	//First release old data
	Release();

	m_szWidth			= szWidth;
	m_szHeight			= szHeight;
	m_szNbComponants	= szNbComponants;

	// We want a one pixel border around the whole atlas to avoid any artefact when sampling texture
	const RT::Vector3i defaultNode(1, 1, szWidth-2);
	m_vNodes.push_back(defaultNode);

    RT::BufferDescriptor descriptor;
	descriptor.addDescriptor(RT::ComponentDescriptor(m_szNbComponants, RT::Type::UnsignedChar, RT::ComponentUsage::Anonyme, false));

	//Build Atlas data
	const size_t szSize = szWidth*szHeight;
	unsigned char* pData = (unsigned char*)m_Buffer.create(descriptor, szSize);
	memset(pData, 0 , sizeof(unsigned char)*szSize);
}

// ----------------------------------------------- texture_atlas_set_region ---
void GUIFontAtlas::CopyRegion(const size_t x, const size_t y, const size_t width, const size_t height, const unsigned char * data, const size_t stride)
{
	MouCa::assertion(data != NULL);
	MouCa::assertion(m_szWidth > 0 && m_szHeight > 0);
	MouCa::assertion(x > 0 && y > 0);
	MouCa::assertion(x < (m_szWidth - 1));
	MouCa::assertion((x + width) <= (m_szWidth - 1));
	MouCa::assertion(y < (m_szHeight - 1));
	MouCa::assertion((y + height) <= (m_szHeight - 1));

	unsigned char* pData = static_cast<unsigned char*>(m_Buffer.getData());

	size_t depth = m_szNbComponants;
	size_t charsize = sizeof(char);
	for(size_t i=0; i<height; ++i)
	{
		memcpy( pData+((y+i)*m_szWidth + x ) * charsize * depth, 
				data + (i*stride) * charsize, width * charsize * depth  );
	}
}

int GUIFontAtlas::Fit(const size_t index, const size_t width, const size_t height) const
{
	int x, y, width_left;
	size_t i;

	const RT::Vector3i& nodeId = m_vNodes[index];
	x = nodeId.x;
	y = nodeId.y;
	width_left = (int)width;
	i = index;

	if ( (x + width) > (m_szWidth-1) )
	{
		return -1;
	}
	while( width_left > 0 )
	{
		const RT::Vector3i& node = m_vNodes[i];
		if( node.y > y )
		{
			y = node.y;
		}
		if( (y + height) > (m_szHeight-1) )
		{
			return -1;
		}
		width_left -= node.z;
		++i;
	}
	return y;
}

void GUIFontAtlas::Merge()
{
	MouCa::assertion(!m_vNodes.empty());

// 	NodesVector::iterator itNode = m_vNodes.begin();
// 	NodesVector::iterator itNext = itNode+1;
// 	while(itNext != m_vNodes.end())
// 	{		
// 		if(itNode->y == itNext->y)
// 		{
// 			itNode->z += itNext->z;
// 			//itNext = m_vNodes.erase(itNext);
// 			itNode = m_vNodes.erase(itNext);
// 		}
// 		else
// 		{
// 			itNode = itNext;
// 			++itNext;
// 		}
// 	}
	for(size_t szNodeID=0; szNodeID < m_vNodes.size()-1; ++szNodeID)
	{
        RT::Vector3i& node = m_vNodes[szNodeID];
        RT::Vector3i& next = m_vNodes[szNodeID + 1];
		if(node.y == next.y)
		{
			m_vNodes[szNodeID].z = next.z;
			m_vNodes.erase(m_vNodes.begin() + szNodeID);
			--szNodeID;
		}
	}
}


// ----------------------------------------------- texture_atlas_get_region ---
RT::Vector4i GUIFontAtlas::FindRegion(const size_t width, const size_t height)
{
	RT::Vector4i region(0,0,width,height);
	
	size_t best_height = _I64_MAX;
	size_t best_index  = _I64_MAX;
	size_t best_width  = _I64_MAX;
	for(size_t szNodeID=0; szNodeID<m_vNodes.size(); ++szNodeID)
	{
		int y = Fit(szNodeID, width, height);
		if(y >= 0)
		{
			const RT::Vector3i& node = m_vNodes[szNodeID];
			if(((y + height) < best_height ) || (((y + height) == best_height) && (node.z < best_width)))
			{
				best_height = (y + height);
				best_index = szNodeID;
				best_width = node.z;
				region.x = node.x;
				region.y = y;
			}
		}
	}
   
	if(best_index == -1)
	{
		region.x = -1;
		region.y = -1;
		region.z = 0;
		region.w = 0;
		return region;
	}

	//Insert new node
	const RT::Vector3i newNode(region.x, region.y + height, width);

	NodesVector::iterator itInsert = m_vNodes.begin();
	std::advance(itInsert, best_index);
	NodesVector::iterator itPreview = m_vNodes.insert(itInsert, newNode);
	/*
	NodesVector::iterator itCurrent = itPreview + 1;
	while(itCurrent != m_vNodes.end())
	//for(i = best_index+1; i < self->nodes->size; ++i)
	{
		//node = (ivec3 *) vector_get( self->nodes, i );
		//prev = (ivec3 *) vector_get( self->nodes, i-1 );
		const int iLimit = (itPreview->x + itPreview->z);
		if(itCurrent->x < iLimit)
		{
			int shrink = iLimit - itCurrent->x;
			itCurrent->x += shrink;
			itCurrent->z -= shrink;
			if(itCurrent->z <= 0)
			{
				itPreview = m_vNodes.erase(itCurrent);
				if(itPreview != m_vNodes.end())
				{
					itCurrent = itPreview + 1;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		//itPreview = itCurrent;
		//itCurrent++;
	}
	*/
	for(size_t szNodeID = best_index+1; szNodeID < m_vNodes.size(); ++szNodeID)
	{
		RT::Vector3i& node	= m_vNodes[szNodeID];
		RT::Vector3i& preview = m_vNodes[szNodeID-1];

		const int iPreview = (preview.x + preview.z);
		if(node.x < iPreview)
		{
			int shrink = iPreview - node.x;
			m_vNodes[szNodeID].x += shrink;
			m_vNodes[szNodeID].z -= shrink;
			if(m_vNodes[szNodeID].z <= 0)
			{
				m_vNodes.erase(m_vNodes.begin() + szNodeID);
				--szNodeID;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	Merge();
	m_used += width * height;
	return region;
}