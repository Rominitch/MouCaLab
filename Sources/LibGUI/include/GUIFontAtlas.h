#pragma once

#include <LibRT/include/RTBufferCPU.h>

class GUIFontAtlas
{
	protected:
		using NodesVector = std::vector<RT::Vector3i>;
		NodesVector m_vNodes;

		size_t m_szWidth;
		size_t m_szHeight;
		size_t m_szNbComponants;

		size_t m_used;
		//unsigned char* m_pData;
		RT::BufferCPU m_Buffer;

	public:
		GUIFontAtlas():
		m_szWidth(0), m_szHeight(0), m_szNbComponants(0), m_used(0)
		{}

		virtual ~GUIFontAtlas()
		{
			Release();
		}

		void Initialize(const size_t szWidth, const size_t szHeight, const size_t szNbComponants);

		void Release();

		RT::BufferCPU& GetData()
		{ 
			return m_Buffer;
		}

		void CopyRegion(const size_t x, const size_t y, const size_t width, const size_t height, const unsigned char* data, const size_t stride);

		int Fit(const size_t index, const size_t width, const size_t height) const;

		void Merge();

		RT::Vector4i FindRegion(const size_t width, const size_t height);

		size_t GetWidth() const
		{
			return m_szWidth;
		}

		size_t GetHeight() const
		{
			return m_szHeight;
		}

		size_t GetNbComponants() const
		{
			return m_szNbComponants;
		}
};