#pragma once

namespace GUI
{
    class Font final
    {
        public:
            Font(void) = default;
            ~Font(void) = default;

            ///	Generate a font format to draw text
            ///	@param	strFontTTLUnicode
            ///	@param	FileOutput
            static void CreateTextureFont(const Core::Path& strFontTTLUnicode, Core::FileWrapperBase& FileOutput);
    
        protected:
            struct STextureGlyph
            {
                unsigned int	m_uiGlyphCode;
                glm::uvec2		m_PositionStart;
                glm::uvec2		m_PositionEnd;
                glm::uvec2		m_Size;

                STextureGlyph(const unsigned int uiGlyphCode, const glm::uvec2& PositionStart, const glm::uvec2& PositionEnd, const glm::uvec2& Size) :
                    m_uiGlyphCode(uiGlyphCode), m_PositionStart(PositionStart), m_PositionEnd(PositionEnd), m_Size(Size)
                {}
            };

            static FT_Glyph CreateGlyph(const FT_Face& face, unsigned int iCharCode);
    };
}

class GUIFontAtlasOld
{
    public:
        using GlyphCode = unsigned int;
        struct STextureGlyph
        {
            float m_uvCoordinate[3];
            float m_uvSize[2];
        };

    protected:
        std::map<GlyphCode, STextureGlyph> m_mapDictionnary;

    public:
        GUIFontAtlasOld()
        {}

        virtual ~GUIFontAtlasOld()
        {}

        void CreateFont(const Core::FileWrapperBase& FontFile);

        void Draw();
        
};