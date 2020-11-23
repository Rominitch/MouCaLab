#pragma once

class GUIFontFile
{
	protected:
		uint64_t			 m_uiStartFile;
		uint64_t			 m_uiStartHeader;
		float				 m_fVersion;
		Core::FileWrapperBase* m_pFile;

	public:
		GUIFontFile(void);
		virtual ~GUIFontFile(void);

		void Initialize(Core::FileWrapperBase& file, const uint64_t uiStartFile=0);

		void Write();
};

