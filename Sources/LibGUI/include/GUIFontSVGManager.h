#pragma once

namespace RT
{
    class BufferCPU;
}

namespace GUI
{
    class Outline;
    class FontSVGManager;

    //----------------------------------------------------------------------------
    /// \brief Manage a family of fonts to draw glyph.
    ///
    class FontFamilySVG
    {
        public:
            struct GlyphSVG
            {
                Outline     _outline;
                float       _advance;
                uint32_t  _glyphID;       ///< Id into font buffer

                GlyphSVG(Outline&& outline=Outline(), float advance=0.0f, const uint32_t glyphID=0):
                _outline(std::move(outline)), _advance(advance), _glyphID(glyphID)
                {}
            };

            using GlyphCode = char32_t;
            using MapGlyph = std::map<GlyphCode, GlyphSVG>;

            FontFamilySVG(FontSVGManager& manager, const std::vector<Core::Path>& fontPaths):
            _manager(manager)
            {
                _fonts.reserve(fontPaths.size());
                for(const auto& path : fontPaths)
                {
                    _fonts.emplace_back(std::move(FontFace(path)));
                }                
            }

            ~FontFamilySVG() = default;

            void initialize(FT_Library library);
            void release();

            bool isNull() const;

            const GlyphSVG& addGlyph(const GlyphCode& glyph);

            const GlyphSVG& readGlyph(const GlyphCode& glyph) const;

            bool tryGlyph(const GlyphCode& glyphCode, FontFamilySVG::GlyphSVG& glyphSVG);

            const MapGlyph& getGlyphs() const   { return _glyphs; }
            MapGlyph&       getEditGlyphs()     { return _glyphs; }

        private:
            struct FontFace
            {
                const Core::Path  _fontsPath;
                FT_Face         _face;
                const bool      _isOTF;

                FontFace(const Core::Path& fontsPath):
                _fontsPath(fontsPath), _face(nullptr), _isOTF(fontsPath.extension() == Core::Path(u8".otf"))
                {}
            };

            FontSVGManager&       _manager;

            std::vector<FontFace> _fonts;       ///< All ordered fonts to find glyph.
            MapGlyph              _glyphs;      ///< Cache of glyphs.
    };

    using FontFamilySVGSPtr = std::shared_ptr<FontFamilySVG>;
    using FontFamilySVGWPtr = std::weak_ptr<FontFamilySVG>;

    class FontSVGManager
    {
        MOUCA_NOCOPY_NOMOVE(FontSVGManager);

        public:
            using FontId = uint16_t;

            FontSVGManager();
            ~FontSVGManager();

            void initialize();
            void release();

            bool isNull() const;

            FontFamilySVGWPtr createFont(const std::vector<Core::Path>& fontPath);
            void buildFontBuffers(RT::BufferCPU& glyphDict, RT::BufferCPU& glyphCells, RT::BufferCPU& glyphPoints);
            void buildFontBuffersV2(RT::BufferCPU& glyphDict, RT::BufferCPU& glyphPoints, RT::BufferCPU& glyphControl);

            std::vector<FontFamilySVG::GlyphSVG*> _ordered;

        private:
            FT_Library                     _library;    ///< FreeType2 library system.
            std::vector<FontFamilySVGSPtr> _fonts;      ///< [OWNERSHIP] List of family font.
            
    };
}