#pragma once

#include <libgui/include/GUIFontAtlas.h>

class GUIFontGlyph
{
	public:
		enum EOutlineType
		{
			None	= 0,
			Line	= 1,
			Inner	= 2,
			Outer	= 3
		};
		struct SGlyphBitmap
		{
			FT_Glyph	m_ftGlyph;
			FT_Bitmap	m_ft_bitmap;
			int			m_ft_bitmap_width;
			int			m_ft_bitmap_rows;
			int			m_ft_bitmap_pitch;
			int			m_ft_glyph_top;
			int			m_ft_glyph_left;

			SGlyphBitmap():
			m_ftGlyph(NULL), m_ft_bitmap_width(0), m_ft_bitmap_rows(0), m_ft_bitmap_pitch(0), m_ft_glyph_top(0), m_ft_glyph_left(0)
			{}

			void Release();
		};
	protected:
	
		char			m_cCharcode;
		//unsigned int	m_id;

		size_t m_width;
		size_t m_height;

		//RT::Vector2i m_Offset;
		RT::Vector4i m_GlyphRect;

		RT::Vector2	m_advance;

		RT::Vector2 m_TexCoordMin;
		RT::Vector2 m_TexCoordMax;
	
		std::map<wchar_t, float> m_mapKerning; ///! Space between the others glyph and this
		
		EOutlineType	m_outline_type;
		float			m_outline_thickness;

	public:
		GUIFontGlyph(void);
		virtual ~GUIFontGlyph(void);
		
		void Initialize(const char wCode, const size_t szRegionWidth, const size_t szRegionHeight, const std::shared_ptr<GUIFontAtlas>& atlas, const RT::Vector4i& region, const SGlyphBitmap& glyphBitmap, const EOutlineType iOutline_type, const float fOutline_thickness);
		void CreateSpecialGlyph(const std::shared_ptr<GUIFontAtlas>& pAtlas, const RT::Vector4i& region);

		void SetAdvancedParameters(const float fAdvancedX, const float fAdvancedY)
		{
			m_advance = RT::Vector2(fAdvancedX, fAdvancedY);
		}
		const RT::Vector2& GetAvanced() const
		{
			return m_advance;
		}

		char GetCharCode() const
		{
			return m_cCharcode;
		}

		const RT::Vector2& GetTexCoordMin() const 
		{
			return m_TexCoordMin;
		}

		const RT::Vector2& GetTexCoordMax() const 
		{
			return m_TexCoordMax;
		}

		const RT::Vector4i& GetRect() const 
		{
			return m_GlyphRect;
		}

		void ClearKerning()
		{
			m_mapKerning.clear();
		}

		void AddKerning(const wchar_t cChar, const float fSpace)
		{
			m_mapKerning[cChar] = fSpace;
		}

		float GetKerning(const wchar_t cChar) const
		{
			float fKerning=0.0f;
			std::map<wchar_t, float>::const_iterator itKerning = m_mapKerning.find(cChar);
			if(itKerning!=m_mapKerning.cend())
			{
				fKerning = itKerning->second;
			}
			return fKerning;
		}

		bool operator< (const GUIFontGlyph& test) const
		{
			return m_cCharcode < test.m_cCharcode;
		}

		bool CheckOutline(const EOutlineType eOutlineType, const float fOutlineThickness) const
		{
			return m_outline_type == eOutlineType && m_outline_thickness == fOutlineThickness;
		}
};

class GUIFontPolice
{
	protected:
		struct SMetricData
		{
			float m_fHeight;
			float m_fAscender;
			float m_fDescender;
			float m_fLinegap;

			SMetricData():
			m_fHeight(0.0f), m_fAscender(0.0f), m_fDescender(0.0f), m_fLinegap(0.0f)
			{}

			SMetricData(const float fHeight, const float fAscender, const float fDescender):
			m_fHeight(fHeight), m_fAscender(fAscender), m_fDescender(fDescender), m_fLinegap(fHeight - fAscender + fDescender)	
			{}
		};

		using GlyphMap = std::map<char, GUIFontGlyph>;
		GlyphMap m_mapGlyphs;

		static const float m_fHResolution;
		static const float m_fInvDoubleHResolution;

		std::shared_ptr<GUIFontAtlas>	m_pAtlas;
		Core::Path		m_strFilename;
		
		float			m_fSize;
		int				m_hinting;
		GUIFontGlyph::EOutlineType	m_outline_type;
		float			m_outline_thickness;
		int				m_filtering;
		int				m_kerning;
		unsigned char	m_lcd_weights[5];

		SMetricData		m_MetricData;
		
		float			m_underline_position;
		float			m_underline_thickness;

		FT_Library		m_pLibrary;
		FT_Face			m_pFace;

		FT_Int32 GetFlag() const;

		void LoadFace(FT_Library& pLibrary, const Core::Path& strFilename, const float size, FT_Face& pFace);
		
		GUIFontGlyph::SGlyphBitmap ConvertGlyphToBitmap(const FT_GlyphSlot& slot) const;
		void CreateSpecialGlyph();
		void GenerateKerning();
	public:
		GUIFontPolice();
		virtual ~GUIFontPolice();

		void Create(const Core::Path& strFilename, const float fSize, std::shared_ptr<GUIFontAtlas>& pAtlas);

		void Release();
		
		void LoadGlyphs(const Core::String& strCharcodes);

		const GUIFontGlyph& GetGlyph(const char wCharcode);

		void SetOutlineParameters(const GUIFontGlyph::EOutlineType eOutlineType, const float fOutlineThickness)
		{
			m_outline_type = eOutlineType;
			m_outline_thickness = fOutlineThickness;
		}
};

/*
  texture_glyph_t *
  texture_font_get_glyph( texture_font_t * self,
						  wchar_t charcode );


size_t texture_font_load_glyphs( texture_font_t * self,
							const wchar_t * charcodes );

float 
texture_glyph_get_kerning( const texture_glyph_t * self,
						   const wchar_t charcode );


texture_glyph_t *
texture_glyph_new( void );
*/