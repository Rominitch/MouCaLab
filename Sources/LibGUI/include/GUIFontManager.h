#pragma once

class GUIFontAtlas;
class GUIFontPolice;

using GUIFontPolicePtr = std::weak_ptr<GUIFontPolice>;

class GUIFontManager
{
	protected:
		using GUIFontsMap = std::map<Core::StringOS, std::shared_ptr<GUIFontPolice>>;
		GUIFontsMap						m_mapFonts;

		std::shared_ptr<GUIFontAtlas>	m_pAtlas;
		Core::String						m_strDefaultCache;
		
	public:
		GUIFontManager(void);
		virtual ~GUIFontManager(void);

		void Create(const size_t szWidth, const size_t szHeight);

		GUIFontPolicePtr BuildFontFromFilename(const Core::StringOS& strFilename, const float size);

		std::shared_ptr<GUIFontAtlas> GetAtlas() const
		{
			return m_pAtlas;
		}

	#ifndef NDEBUG
		void WritePictureAtlas();
	#endif
};