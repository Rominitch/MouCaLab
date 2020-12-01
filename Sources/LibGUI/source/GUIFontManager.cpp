#include "Dependencies.h"

#include <LibGUI/include/GUIFontAtlas.h>
#include <LibGUI/include/GUIFontGlyph.h>
#include <LibGUI/include/GUIFontManager.h>

GUIFontManager::GUIFontManager(void):
m_pAtlas(new GUIFontAtlas())
{}

GUIFontManager::~GUIFontManager(void)
{}

void GUIFontManager::Create(const size_t szWidth, const size_t szHeight)
{
    m_pAtlas->Initialize(szWidth, szHeight, 1);
    
    m_mapFonts.clear();
}

// ----------------------------------------- font_manager_get_from_filename ---
GUIFontPolicePtr GUIFontManager::BuildFontFromFilename(const Core::StringOS& strFilename, const float size)
{
    MOUCA_ASSERT(!strFilename.empty());
    MOUCA_ASSERT(size > 0);

    GUIFontPolicePtr pDemandedFont;

    GUIFontsMap::const_iterator itFont = m_mapFonts.find(strFilename);
    if(itFont==m_mapFonts.cend())
    {
        //Build font
        std::shared_ptr<GUIFontPolice> pFont = std::shared_ptr<GUIFontPolice>(new GUIFontPolice());
        pFont->Create(strFilename, size, m_pAtlas);

        //Load default cache
        pFont->LoadGlyphs(m_strDefaultCache);

        //Set information to manager
        m_mapFonts[strFilename] = pFont;
        pDemandedFont = pFont;
    }
    else
    {
        pDemandedFont = itFont->second;
    }

    return pDemandedFont;
}

#ifndef NDEBUG
#include <LibRT/include/RTImage.h>

void GUIFontManager::WritePictureAtlas()
{
    /*
    RT::BufferCPU buffer;
    RT::BufferDescriptor bufferDescriptor;
    bufferDescriptor.addDescriptor(RT::Descriptor(3, RT::TypeUnsignedChar, RT::BufferDefault));
    const size_t szNbPixels = m_pAtlas->GetWidth()*m_pAtlas->GetHeight();
    unsigned char* pData = (unsigned char*)buffer.create(bufferDescriptor, szNbPixels);		
    unsigned char* pDataAtlas = (unsigned char*)m_pAtlas->GetData().getData();
    for(size_t szPixel=0; szPixel<szNbPixels; ++szPixel)
    {
        const size_t szPixelEnd = szPixel*3;
        pData[szPixelEnd]	= pDataAtlas[szPixel];
        pData[szPixelEnd+1] = pDataAtlas[szPixel];
        pData[szPixelEnd+2] = pDataAtlas[szPixel];
    }

    RT::Image fontImage;
    fontImage.createImage(buffer, m_pAtlas->GetWidth(), m_pAtlas->GetHeight());
        
    Core::StringStreamOS ssfile;
    size_t szID=0;
    do 
    {
        ssfile.str(L"");
        ssfile << L"Generated/GUIAtlas_" << szID << L".png";
        ++szID;
    }
    while(Core::File::isExist(ssfile.str()));		

    fontImage.saveImage(ssfile.str());

// 	BTImage fontImage;
// 	fontImage.CreateImage(m_pAtlas->GetData(), m_pAtlas->GetWidth(), m_pAtlas->GetHeight());
// 	fontImage.SaveImage(L"GUIFontManager_fontDebug.png");
    */
}
#endif
/*
// ----------------------------------------- font_manager_get_from_description ---
texture_font_t *
font_manager_get_from_description( font_manager_t *self,
                                   const char * family,
                                   const float size,
                                   const int bold,
                                   const int italic )
{
    texture_font_t *font;
    char *filename = 0;

    assert( self );

    if( file_exists( family ) )
    {
        filename = strdup( family );
    }
    else
    {
#ifdef MOUCA_OS_WINDOWS
        fprintf( stderr, "\"font_manager_get_from_description\" not implemented yet.\n" );
        return 0;
#endif
        filename = font_manager_match_description( self, family, size, bold, italic );
        if( !filename )
        {
            fprintf( stderr, "No \"%s (size=%.1f, bold=%d, italic=%d)\" font available.\n",
                     family, size, bold, italic );
            return 0;
        }
    }
    font = font_manager_get_from_filename( self, filename, size );

    free( filename );
    return font;
}

// ------------------------------------------- font_manager_get_from_markup ---
texture_font_t *
font_manager_get_from_markup( font_manager_t *self,
                              const markup_t *markup )
{
    assert( self );
    assert( markup );

    return font_manager_get_from_description( self, markup->family, markup->size,
                                              markup->bold,   markup->italic );
}
/*
// ----------------------------------------- font_manager_match_description ---
char *
font_manager_match_description( font_manager_t * self,
                                const char * family,
                                const float size,
                                const int bold,
                                const int italic )
{
// Use of fontconfig is disabled by default.
#if 1
    return 0;
#else
#  ifdef MOUCA_OS_WINDOWS
      fprintf( stderr, "\"font_manager_match_description\" not implemented for windows.\n" );
      return 0;
#  endif
    char *filename = 0;
    int weight = FC_WEIGHT_REGULAR;
    int slant = FC_SLANT_ROMAN;
    if ( bold )
    {
        weight = FC_WEIGHT_BOLD;
    }
    if( italic )
    {
        slant = FC_SLANT_ITALIC;
    }
    FcInit();
    FcPattern *pattern = FcPatternCreate();
    FcPatternAddDouble( pattern, FC_SIZE, size );
    FcPatternAddInteger( pattern, FC_WEIGHT, weight );
    FcPatternAddInteger( pattern, FC_SLANT, slant );
    FcPatternAddString( pattern, FC_FAMILY, (FcChar8*) family );
    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        fprintf( stderr, "fontconfig error: could not match family '%s'", family );
        return 0;
    }
    else
    {
        FcValue value;
        FcResult result = FcPatternGet( match, FC_FILE, 0, &value );
        if ( result )
        {
            fprintf( stderr, "fontconfig error: could not match family '%s'", family );
        }
        else
        {
            filename = strdup( (char *)(value.u.s) );
        }
    }
    FcPatternDestroy( match );
    return filename;
#endif
}
*/