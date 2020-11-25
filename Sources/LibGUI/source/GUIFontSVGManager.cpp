#include "Dependancies.h"

#include "LibRT/include/RTBufferCPU.h"
#include "LibRT/include/RTBufferDescriptor.h"

#include "LibGUI/include/GUIOutline.h"
#include "LibGUI/include/GUIFontSVGManager.h"

namespace GUI
{

FontSVGManager::FontSVGManager():
_library(nullptr)
{
    MOUCA_PRE_CONDITION(isNull());
}

FontSVGManager::~FontSVGManager()
{
    MOUCA_POST_CONDITION(isNull());
}

void FontSVGManager::initialize()
{
    MOUCA_PRE_CONDITION(isNull());

    if (FT_Init_FreeType(&_library))
    {
        BT_THROW_ERROR(u8"GUI", u8"FreeType2InitError");
    }
    
    MOUCA_POST_CONDITION(!isNull());
}

void FontSVGManager::release()
{
    _ordered.clear();

    // Release fonts
    for (auto& font : _fonts)
    {
        font->release();
    }
    _fonts.clear();

    if (FT_Done_FreeType(_library))
    {
        BT_THROW_ERROR(u8"GUI", u8"FreeType2DoneError");
    }
    _library = nullptr;
}

bool FontSVGManager::isNull() const
{
    return _library == nullptr && _fonts.empty();
}

FontFamilySVGWPtr FontSVGManager::createFont(const std::vector<Core::Path>& fontPath)
{
    auto font = std::make_shared<FontFamilySVG>(*this, fontPath);
    font->initialize(_library);
    return _fonts.emplace_back(std::move(font));
}

struct GlyphInstance
{
    BoundingBox2D rect;
    alignas(4) uint32_t  glyph_index;
    alignas(4) float       sharpness;

    GlyphInstance() :
    glyph_index(0), sharpness(1.0f)
    {}
};
struct fd_CellInfo
{
    uint32_t point_offset;
    uint32_t cell_offset;
    uint32_t cell_count_x;
    uint32_t cell_count_y;
};

struct DeviceGlyphInfo
{
    BoundingBox2D bbox;
    uint32_t point_offset;
    uint32_t cell_offset;
    uint32_t cell_count_x;
    uint32_t cell_count_y;
};

struct GlyphInfo
{
    alignas(16) BoundingBox2D _bbox;
    alignas(8)  glm::uvec2    _indices; // [Start-End[ indices

    GlyphInfo(const BoundingBox2D& bbox, const uint32_t& start, const uint32_t& end):
    _bbox(bbox), _indices(start, end)
    {}
};

void FontSVGManager::buildFontBuffers(RT::BufferCPU& glyphDict, RT::BufferCPU& glyphCells, RT::BufferCPU& glyphPoints)
{
    BT_ASSERT(!_fonts.empty());

    uint32_t nbGlyphs = 0;
    uint32_t nbCells  = 0;
    uint32_t nbPoints = 0;
    // Compute stats
    for (auto& font : _fonts)
    {
        nbGlyphs += static_cast<uint32_t>(font->getGlyphs().size());
        for (auto& glyph : font->getGlyphs())
        {
            nbCells  += static_cast<uint32_t>(glyph.second._outline.getCells().size());
            nbPoints += static_cast<uint32_t>(glyph.second._outline.getPoints().size());
        }
    }
    BT_ASSERT(nbGlyphs > 0);

    auto glyphs = reinterpret_cast<DeviceGlyphInfo*>(glyphDict.create(RT::BufferDescriptor(sizeof(DeviceGlyphInfo)), nbGlyphs));
    auto cells  = reinterpret_cast<uint32_t*>(glyphCells.create(RT::BufferDescriptor(sizeof(uint32_t)),          nbCells));
    auto points = reinterpret_cast<glm::vec2*>(glyphPoints.create(RT::BufferDescriptor(sizeof(glm::vec2)),           nbPoints));
    
    uint32_t pointOffset = 0;
    uint32_t cellOffset  = 0;

    for (auto& glyph : _ordered)
    {
        glyphs->bbox = glyph->_outline.getBoundingBox();
        glyphs->point_offset = pointOffset;
        glyphs->cell_offset  = cellOffset;
        glyphs->cell_count_x = glyph->_outline.getCellCountX();
        glyphs->cell_count_y = glyph->_outline.getCellCountY();
        ++glyphs;

        memcpy(cells,  glyph->_outline.getCells().data(),  sizeof(uint32_t)  * glyph->_outline.getCells().size());
        cellOffset  += static_cast<uint32_t>(glyph->_outline.getCells().size());
        cells       += glyph->_outline.getCells().size();

        memcpy(points, glyph->_outline.getPoints().data(), sizeof(glm::vec2) * glyph->_outline.getPoints().size());
        pointOffset += static_cast<uint32_t>(glyph->_outline.getPoints().size());
        points      += glyph->_outline.getPoints().size();
    }
}

void FontSVGManager::buildFontBuffersV2(RT::BufferCPU& glyphDict, RT::BufferCPU& glyphPoints, RT::BufferCPU& glyphControl)
{
    BT_ASSERT(!_fonts.empty());

    uint32_t nbGlyphs   = 0;
    uint32_t nbControls = 0;
    uint32_t nbPoints   = 0;

    // Compute stats
    for (auto& font : _fonts)
    {
        nbGlyphs += static_cast<uint32_t>(font->getGlyphs().size());
        for (auto& glyph : font->getGlyphs())
        {
            nbPoints   += static_cast<uint32_t>(glyph.second._outline.getOutlinePoint().size());
            nbControls += static_cast<uint32_t>(glyph.second._outline.getControlPoint().size());
        }
    }
    BT_ASSERT(nbGlyphs > 0);

    auto glyphs   = reinterpret_cast<GlyphInfo*>            (glyphDict.create(RT::BufferDescriptor(sizeof(GlyphInfo)),                nbGlyphs));
    auto points   = reinterpret_cast<Outline::Point*>       (glyphPoints.create(RT::BufferDescriptor(sizeof(Outline::Point)),         nbPoints));
    auto controls = reinterpret_cast<Outline::ControlPoint*>(glyphControl.create(RT::BufferDescriptor(sizeof(Outline::ControlPoint)), nbControls));

    BT_ASSERT(_ordered.size() == nbGlyphs);

    uint32_t pointOffset   = 0;
    uint32_t controlOffset = 0;
    for (auto& glyph : _ordered)
    {
        const uint32_t nextPoints = static_cast<uint32_t>(glyph->_outline.getOutlinePoint().size());
        *glyphs = std::move(GlyphInfo(glyph->_outline.getBoundingBox(), pointOffset, pointOffset + nextPoints));
        ++glyphs;

        // Compute Points buffer
        for (const auto& point : glyph->_outline.getOutlinePoint())
        {
            const Outline::Code code = point.getCode();
            const uint32_t index = code == Outline::Code::EndCurve
                                   ? point.getIndex() + pointOffset
                                   : point.getIndex() + controlOffset;
            *points = std::move(Outline::Point(point._point, code, index));
            ++points;
        }

        // Compute ControlPoints buffer
        memcpy(controls, glyph->_outline.getControlPoint().data(), sizeof(Outline::ControlPoint) * glyph->_outline.getControlPoint().size());
        controls      += glyph->_outline.getControlPoint().size();
        controlOffset += static_cast<uint32_t>(glyph->_outline.getControlPoint().size());

        pointOffset += nextPoints;
    }
}

void FontFamilySVG::initialize(FT_Library library)
{
    MOUCA_PRE_CONDITION(library != nullptr);
    MOUCA_PRE_CONDITION(isNull());

    for(auto& font : _fonts)
    {
        if (FT_New_Face(library, font._fontsPath.string().c_str(), 0, &font._face))
        {
            BT_THROW_ERROR(u8"GUI", u8"FreeType2NewFaceError");
        }

        FT_Error error = FT_Select_Charmap(font._face, ft_encoding_unicode);
        BT_ASSERT(!error);
        //error = FT_Set_Char_Size(font._face, 0, 64000, 96, 96);
        error = FT_Set_Char_Size(font._face, 0, 6400, 96, 96);
        BT_ASSERT(!error);
    }

    MOUCA_POST_CONDITION(!isNull());
}

void FontFamilySVG::release()
{
    MOUCA_PRE_CONDITION(!isNull());

    _glyphs.clear();

    for (auto& font : _fonts)
    {
        if (FT_Done_Face(font._face))
        {
            BT_THROW_ERROR(u8"GUI", u8"FreeType2DoneFaceError");
        }
    }
    _fonts.clear();

    MOUCA_POST_CONDITION(isNull());
}

bool FontFamilySVG::isNull() const
{
    for (const auto& font : _fonts)
    {
        if (font._face != nullptr)
            return false;
    }
    return _glyphs.empty();
}

const FontFamilySVG::GlyphSVG& FontFamilySVG::addGlyph(const GlyphCode& glyphCode)
{
    MOUCA_PRE_CONDITION(_glyphs.find(glyphCode) == _glyphs.cend());
    
    FT_UInt glyph_index = 0;
    auto itFont = _fonts.cbegin();

    const auto& createGlyph = [&](const auto& itFont) -> const FontFamilySVG::GlyphSVG&
    {
        if (FT_Load_Glyph(itFont->_face, glyph_index, FT_LOAD_NO_HINTING))
        {
            BT_THROW_ERROR(u8"GUI", u8"FreeType2LoadGlyphError");
        }
        Outline outline;
        if (itFont->_isOTF)
        {
            FT_Outline_Reverse(&itFont->_face->glyph->outline);
        }   
        outline.convert(itFont->_face->glyph->outline);

        const auto [it, success] = _glyphs.insert({ glyphCode, std::move(GlyphSVG(std::move(outline), static_cast<float>(itFont->_face->glyph->metrics.horiAdvance * Outline::_scale), static_cast<uint32_t>(_manager._ordered.size()))) });
        BT_ASSERT(success);
        _manager._ordered.emplace_back(&it->second);
        return it->second;
    };

    // Parse each font to find where is glyph
    while(itFont != _fonts.cend())
    {
        glyph_index = FT_Get_Char_Index(itFont->_face, glyphCode);
        if (glyph_index != 0)
        {
            return createGlyph(itFont);
        }
        ++itFont;
    }

    // Nothing find compute empty symbol using one font.
    return createGlyph(_fonts.cbegin());
}

const FontFamilySVG::GlyphSVG& FontFamilySVG::readGlyph(const GlyphCode& glyph) const
{
    MOUCA_PRE_CONDITION(!isNull());
    MOUCA_PRE_CONDITION(_glyphs.find(glyph) != _glyphs.cend());
    return _glyphs.find(glyph)->second;
}

bool FontFamilySVG::tryGlyph(const GlyphCode& glyphCode, FontFamilySVG::GlyphSVG& glyphSVG)
{
    MOUCA_PRE_CONDITION(!isNull());
    
    auto itGlyph = _glyphs.find(glyphCode);
    if(itGlyph == _glyphs.cend())
    {
        glyphSVG = addGlyph(glyphCode);
        return true;
    }
    else
    {
        glyphSVG = _glyphs.find(glyphCode)->second;
    }
    return false;
}

}