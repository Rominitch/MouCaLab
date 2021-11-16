#include "Dependencies.h"
#include <LibGUI/include/GUIFontGlyph.h>

#define SPECIAL_GLYPH (char)(-1)

GUIFontGlyph::GUIFontGlyph(void):
m_width(0), m_height(0), m_outline_type(None), m_outline_thickness(1.0f)
{}

GUIFontGlyph::~GUIFontGlyph(void)
{}

void GUIFontGlyph::Initialize(const char wCode, const size_t szRegionWidth, const size_t szRegionHeight, const std::shared_ptr<GUIFontAtlas>& pAtlas, const RT::Vector4i& region, const GUIFontGlyph::SGlyphBitmap& glyphBitmap, const GUIFontGlyph::EOutlineType iOutline_type, const float fOutline_thickness)
{
	const float fInvWidth = 1.0f/pAtlas->GetWidth();
	const float fInvHeight = 1.0f/pAtlas->GetHeight();

	m_cCharcode		= wCode;
	m_width			= szRegionWidth;
	m_height		= szRegionHeight;
	m_outline_type	= iOutline_type;
	m_outline_thickness = fOutline_thickness;
	//m_Offset		= RT::Vector2i(glyphBitmap.m_ft_glyph_left, glyphBitmap.m_ft_glyph_top);
	m_GlyphRect		= RT::Vector4i(glyphBitmap.m_ft_glyph_left, glyphBitmap.m_ft_glyph_top, glyphBitmap.m_ft_glyph_left+(int)szRegionWidth, glyphBitmap.m_ft_glyph_top-(int)szRegionHeight);
	m_TexCoordMin	= RT::Vector2(region.x*fInvWidth, region.y*fInvHeight);
	m_TexCoordMax	= RT::Vector2((region.x + szRegionWidth)*fInvWidth, (region.y + szRegionHeight)*fInvHeight);
}

void GUIFontGlyph::CreateSpecialGlyph(const std::shared_ptr<GUIFontAtlas>& pAtlas, const RT::Vector4i& region)
{
	const float fInvWidth = 1.0f/pAtlas->GetWidth();
	const float fInvHeight = 1.0f/pAtlas->GetHeight();

	m_cCharcode = SPECIAL_GLYPH;
	m_TexCoordMin		= RT::Vector2((region.x+2)*fInvWidth, (region.x+2)*fInvHeight);
	m_TexCoordMax		= RT::Vector2((region.x+3)*fInvWidth, (region.y+3)*fInvHeight);
}

const float GUIFontPolice::m_fHResolution			= 64.0f;
const float GUIFontPolice::m_fInvDoubleHResolution	= 1.0f/(GUIFontPolice::m_fHResolution*GUIFontPolice::m_fHResolution);

GUIFontPolice::GUIFontPolice(void):
m_pLibrary(NULL), m_pFace(NULL),
m_fSize(0.0f), m_hinting(1), m_outline_type(GUIFontGlyph::None), m_outline_thickness(0.0f),
m_filtering(1), m_kerning(1), m_underline_position(0.0f), m_underline_thickness(0.0f)
{
	m_lcd_weights[0] = 0x10;
	m_lcd_weights[1] = 0x40;
	m_lcd_weights[2] = 0x70;
	m_lcd_weights[3] = 0x40;
	m_lcd_weights[4] = 0x10;
}

GUIFontPolice::~GUIFontPolice(void)
{
	Release();
}

void GUIFontPolice::Release()
{
	if(m_pFace!=NULL)
	{
		FT_Done_Face(m_pFace);
		m_pFace	= NULL;
	}
	if(m_pLibrary!=NULL)
	{
		FT_Done_FreeType(m_pLibrary);
		m_pLibrary	= NULL;
	}	
}

void GUIFontPolice::Create(const Core::StringOS& strFilename, const float fSize, std::shared_ptr<GUIFontAtlas>& pAtlas)
{
	MouCa::assertion(fSize > 0.0f);
	MouCa::assertion(!strFilename.empty());

	//Clear old data
	Release();

	m_pAtlas		= pAtlas;
	m_strFilename	= strFilename;
	m_fSize			= fSize;

	//Load face
	LoadFace(m_pLibrary, m_strFilename, m_fSize, m_pFace);

	//Build all positions
	m_underline_position = m_pFace->underline_position * m_fInvDoubleHResolution * m_fSize;
	m_underline_position = std::round(m_underline_position);
	if(m_underline_position > -2.0)
	{
		m_underline_position = -2.0;
	}

	m_underline_thickness = m_pFace->underline_thickness * m_fInvDoubleHResolution * m_fSize;
	m_underline_thickness = std::round(m_underline_thickness);
	if(m_underline_thickness < 1)
	{
		m_underline_thickness = 1.0;
	}

	//Get metric information
	FT_Size_Metrics metrics = m_pFace->size->metrics; 
	m_MetricData = SMetricData((float)(metrics.ascender >> 6) / 100.0f, (float)(metrics.descender >> 6)	/ 100.0f, (float)(metrics.height >> 6)/ 100.0f);

	CreateSpecialGlyph();
}

void GUIFontPolice::LoadFace(FT_Library& pLibrary, const Core::StringOS& strFilename, const float size, FT_Face& pFace)
{
	const size_t szHRes = 64;	

	//Create And Initialize A FreeType Font Library.
	if(FT_Init_FreeType(&pLibrary))
	{
		throw Core::Exception(Core::ErrorData("ModuleError", "InitializeError"));
	}

	try
	{
        const Core::String u8Filename = Core::convertToU8(strFilename);
		//Create And Initialize A Face.
		if(FT_New_Face(pLibrary, u8Filename.c_str(), 0, &pFace))
		{
			throw Core::Exception(Core::ErrorData("ModuleError", "FontFileMissingError") << u8Filename);
		}

		//Select Charmap for unicode support
		if(FT_Select_Charmap(pFace, FT_ENCODING_UNICODE))
		{
			throw Core::Exception(Core::ErrorData("ModuleError", "FontFileMissingError") << u8Filename);
		}

		// Set char size
		if(FT_Set_Char_Size(pFace, (int)(size*szHRes), 0, 72*szHRes, 72))
		{
			throw Core::Exception(Core::ErrorData("ModuleError", "FontFileMissingError") << u8Filename);
		}

		// Set transform matrix
		FT_Matrix matrix = {	(int)((1.0/m_fHResolution)	* 0x10000L),
								(int)((0.0)					* 0x10000L),
								(int)((0.0)					* 0x10000L),
								(int)((1.0)					* 0x10000L)
							};
		FT_Set_Transform(pFace, &matrix, NULL);
	}
	catch(const Core::Exception&)
	{
		Release();
		throw;
	}
}

void GUIFontPolice::GenerateKerning()
{
	MouCa::assertion(m_pFace != NULL && m_pLibrary != NULL);
	MouCa::assertion(!m_mapGlyphs.empty());

	//Compute start glyph
	GlyphMap::iterator itStartGlyph = m_mapGlyphs.begin();
	//Parse each Glyph
	GlyphMap::iterator itGlyph = itStartGlyph;
	while(itGlyph != m_mapGlyphs.end())
	{
		//Clear glyph kerning
		itGlyph->second.ClearKerning();

		const FT_UInt glyphID = FT_Get_Char_Index(m_pFace, itGlyph->second.GetCharCode());

		GlyphMap::const_iterator itGlyphPreview = itStartGlyph;
		while(itGlyphPreview != m_mapGlyphs.cend())
		{
			const FT_UInt glyphPreviewID = FT_Get_Char_Index(m_pFace, itGlyphPreview->second.GetCharCode());
			
			//Read kerning between the two characters
			FT_Vector kerning;
			if(FT_Get_Kerning(m_pFace, glyphPreviewID, glyphID, FT_KERNING_UNFITTED, &kerning))
			{
				//exception
			}
			
			if(kerning.x > 0)
			{
				itGlyph->second.AddKerning(itGlyphPreview->second.GetCharCode(), kerning.x / (float)(64.0f*64.0f));
			}
			++itGlyphPreview;
		}
		++itGlyph;
	}
}

FT_Int32 GUIFontPolice::GetFlag() const
{
	FT_Int32 flags = 0;
	if(m_outline_type > 0)
	{
		flags |= FT_LOAD_NO_BITMAP;
	}
	else
	{
		flags |= FT_LOAD_RENDER;
	}

	if(m_hinting==0)
	{
		flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
	}
	else
	{
		flags |= FT_LOAD_FORCE_AUTOHINT;
	}

	if(m_pAtlas->GetNbComponants() == 3)
	{
		FT_Library_SetLcdFilter(m_pLibrary, FT_LCD_FILTER_LIGHT);
		flags |= FT_LOAD_TARGET_LCD;
		if(m_filtering != 0)
		{
			FT_Library_SetLcdFilterWeights(m_pLibrary, (unsigned char*)m_lcd_weights);
		}
	}
	return flags;
}

void GUIFontPolice::LoadGlyphs(const Core::String& strCharcodes)
{
	const FT_Int32 flags = GetFlag();

	/* Load each glyph */
	for(size_t szChar=0; szChar<strCharcodes.size(); ++szChar)
	{
		const FT_UInt glyph_index = FT_Get_Char_Index(m_pFace, strCharcodes[szChar]);
		// WARNING: We use texture-atlas depth to guess if user wants LCD subpixel rendering
		if(FT_Load_Glyph(m_pFace, glyph_index, flags))
		{
			//exception
		}

		GUIFontGlyph::SGlyphBitmap glyphBitmap = ConvertGlyphToBitmap(m_pFace->glyph);

		const size_t szRegionWidth	= glyphBitmap.m_ft_bitmap_width/m_pAtlas->GetNbComponants();
		const size_t szRegionHeight	= glyphBitmap.m_ft_bitmap_rows;
		const RT::Vector4i region = m_pAtlas->FindRegion(szRegionWidth+1, szRegionHeight+1);
		if(region.x < 0)
		{
			continue;
		}

		if(glyphBitmap.m_ft_bitmap.buffer!=NULL)
		{
			m_pAtlas->CopyRegion(region.x, region.y, szRegionWidth, szRegionHeight, glyphBitmap.m_ft_bitmap.buffer, glyphBitmap.m_ft_bitmap.pitch);
		}

		GUIFontGlyph glyph;
		glyph.Initialize(strCharcodes[szChar], szRegionWidth, szRegionHeight, m_pAtlas, region, glyphBitmap, m_outline_type, m_outline_thickness);

		// Discard hinting to get advance
		FT_Load_Glyph(m_pFace, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
		glyph.SetAdvancedParameters(m_pFace->glyph->advance.x/m_fHResolution, m_pFace->glyph->advance.y/m_fHResolution);

		m_mapGlyphs[glyph.GetCharCode()] = glyph;

		glyphBitmap.Release();
	}

	//Build Kerning for all loaded glyphs
	GenerateKerning();
}

GUIFontGlyph::SGlyphBitmap GUIFontPolice::ConvertGlyphToBitmap(const FT_GlyphSlot& slot) const
{
	GUIFontGlyph::SGlyphBitmap glyphBitmap;
	if(m_outline_type == GUIFontGlyph::None)
	{
		glyphBitmap.m_ft_bitmap			= slot->bitmap;
		glyphBitmap.m_ft_bitmap_width	= slot->bitmap.width;
		glyphBitmap.m_ft_bitmap_rows	= slot->bitmap.rows;
		glyphBitmap.m_ft_bitmap_pitch	= slot->bitmap.pitch;
		glyphBitmap.m_ft_glyph_top		= slot->bitmap_top;
		glyphBitmap.m_ft_glyph_left		= slot->bitmap_left;
	}
	else
	{
		FT_Stroker stroker;
		if(FT_Stroker_New(m_pLibrary, &stroker))
		{
			MouCa::assertion(false);
			//Exception
		}
		FT_Stroker_Set(stroker, (int)(m_outline_thickness * m_fHResolution), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

		FT_Glyph ft_glyph;
		if(FT_Get_Glyph(slot, &ft_glyph))
		{
			MouCa::assertion(false);
			//Exception
		}
		
		FT_Error error=0;
		switch(m_outline_type)
		{
			case GUIFontGlyph::Line:
			{
				error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
			}
			break;
			case GUIFontGlyph::Inner:
			{
				error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
			}
			break;
			case GUIFontGlyph::Outer:
			{
				error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
			}
			break;
		}
		if(error)
		{
			MouCa::assertion(false);
			//Exception
		}
		  
		if(m_pAtlas->GetNbComponants() == 1)
		{
			error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		}
		else
		{
			error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
		}
		if(error)
		{
			MouCa::assertion(false);
			//Exception
		}
		FT_BitmapGlyph ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
		glyphBitmap.m_ftGlyph		  = ft_glyph;
		glyphBitmap.m_ft_bitmap       = ft_bitmap_glyph->bitmap;
		glyphBitmap.m_ft_bitmap_width = ft_bitmap_glyph->bitmap.width;
		glyphBitmap.m_ft_bitmap_rows  = ft_bitmap_glyph->bitmap.rows;
		glyphBitmap.m_ft_bitmap_pitch = ft_bitmap_glyph->bitmap.pitch;
		glyphBitmap.m_ft_glyph_top    = ft_bitmap_glyph->top;
		glyphBitmap.m_ft_glyph_left   = ft_bitmap_glyph->left;
		FT_Stroker_Done(stroker);
	}
	return glyphBitmap;
 }

const GUIFontGlyph& GUIFontPolice::GetGlyph(const char wCharcode)
{
	//Find glyph
	GlyphMap::iterator itGlyph = m_mapGlyphs.find(wCharcode);
	
	if(itGlyph != m_mapGlyphs.cend() && itGlyph->second.CheckOutline(m_outline_type, m_outline_thickness))
	{
		
	}
	else
	{
		LoadGlyphs(Core::String(1, wCharcode));
		itGlyph = m_mapGlyphs.find(wCharcode);
	}
	return (itGlyph->second);
}

void GUIFontPolice::CreateSpecialGlyph()
{
	//charcode -1 is special : it is used for line drawing (overline, underline, strikethrough) and background.

	const size_t szRegionSize = 4;
	const RT::Vector4i region = m_pAtlas->FindRegion(szRegionSize+1, szRegionSize+1);
	if(region.x < 0)
	{
		//Exception
	}

	const unsigned char data[szRegionSize*szRegionSize*3] =
	{255,255,255,255,255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,255,255,255,255,
	 255,255,255,255,255,255,255,255,255,255,255,255 };

	m_pAtlas->CopyRegion(region.x, region.y, szRegionSize, szRegionSize, data, 0);

	GUIFontGlyph glyph;
	glyph.CreateSpecialGlyph(m_pAtlas, region);
		
	m_mapGlyphs[glyph.GetCharCode()] = glyph;
}
/*
texture_glyph_t* LoadGlyph( const char *  filename,     const wchar_t charcode,
			const float   highres_size, const float   lowres_size,
			const float   padding )
{
	size_t i, j;
	FT_Library library;
	FT_Face face;

	FT_Init_FreeType( &library );
	FT_New_Face( library, filename, 0, &face );
	FT_Select_Charmap( face, FT_ENCODING_UNICODE );
	FT_UInt glyph_index = FT_Get_Char_Index( face, charcode );

	// Render glyph at high resolution (highres_size points)
	FT_Set_Char_Size( face, highres_size*64, 0, 72, 72 );
	FT_Load_Glyph( face, glyph_index,
				   FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
	FT_GlyphSlot slot = face->glyph;
	FT_Bitmap bitmap = slot->bitmap;

	// Allocate high resolution buffer
	size_t highres_width  = bitmap.width + 2*padding*highres_size;
	size_t highres_height = bitmap.rows + 2*padding*highres_size;
	double * highres_data = (double *) malloc( highres_width*highres_height*sizeof(double) );
	memset( highres_data, 0, highres_width*highres_height*sizeof(double) );

	// Copy high resolution bitmap with padding and normalize values
	for( j=0; j < bitmap.rows; ++j )
	{
		for( i=0; i < bitmap.width; ++i )
		{
			int x = i + padding;
			int y = j + padding;
			highres_data[y*highres_width+x] = bitmap.buffer[j*bitmap.width+i]/255.0;
		}
	}

	// Compute distance map
	distance_map( highres_data, highres_width, highres_height );

	// Allocate low resolution buffer
	size_t lowres_width  = round(highres_width * lowres_size/highres_size);
	size_t lowres_height = round(highres_height * lowres_width/(float) highres_width);
	double * lowres_data = (double *) malloc( lowres_width*lowres_height*sizeof(double) );
	memset( lowres_data, 0, lowres_width*lowres_height*sizeof(double) );

	// Scale down highres buffer into lowres buffer
	resize( highres_data, highres_width, highres_height,
			lowres_data,  lowres_width,  lowres_height );

	// Convert the (double *) lowres buffer into a (unsigned char *) buffer and
	// rescale values between 0 and 255.
	unsigned char * data =
		(unsigned char *) malloc( lowres_width*lowres_height*sizeof(unsigned char) );
	for( j=0; j < lowres_height; ++j )
	{
		for( i=0; i < lowres_width; ++i )
		{
			double v = lowres_data[j*lowres_width+i];
			data[j*lowres_width+i] = (int) (255*(1-v));
		}
	}

	// Compute new glyph information from highres value
	float ratio = lowres_size / highres_size;
	size_t pitch  = lowres_width * sizeof( unsigned char );

	// Create glyph
	texture_glyph_t * glyph = texture_glyph_new( );
	glyph->offset_x = (slot->bitmap_left + padding*highres_width) * ratio;
	glyph->offset_y = (slot->bitmap_top + padding*highres_height) * ratio;
	glyph->width    = lowres_width;
	glyph->height   = lowres_height;
	glyph->charcode = charcode;

	ivec4 region = texture_atlas_get_region( atlas, glyph->width, glyph->height );
   
	texture_atlas_set_region( atlas, region.x, region.y, glyph->width, glyph->height, data, pitch );
	glyph->s0       = region.x/(float)atlas->width;
	glyph->t0       = region.y/(float)atlas->height;
	glyph->s1       = (region.x + glyph->width)/(float)atlas->width;
	glyph->t1       = (region.y + glyph->height)/(float)atlas->height;

	FT_Load_Glyph( face, glyph_index,
				   FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
	glyph->advance_x = ratio * face->glyph->advance.x/64.0;
	glyph->advance_y = ratio * face->glyph->advance.y/64.0;

	free( highres_data );
	free( lowres_data );
	free( data );

	return glyph;
}
*/
void GUIFontGlyph::SGlyphBitmap::Release()
{
	//Clear data inside
	if(m_ftGlyph!=NULL)
	{
		FT_Done_Glyph(m_ftGlyph);
		m_ftGlyph=NULL;
	}
}