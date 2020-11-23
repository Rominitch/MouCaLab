/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace RT
{
    class Copy
    {
	    protected:
		    Copy(void)
		    {}

	    public:
		    virtual ~Copy(void)
		    {}

		    virtual void copy(void* pDestination, const void* pSource) const = 0;
    };

    class Copy1D : public Copy
    {
	    public:
		    enum ECopyInfo
		    {
			    DestinationOffset,
			    DestinationSize,
			    SourceOffset,			
			    SourceSize,
			    CopySize,
			    NbInformations
		    };

	    protected:
		    size_t m_szInformation[NbInformations];

	    public:
		    Copy1D()
		    {
			    memset(m_szInformation, 0, sizeof(size_t)*NbInformations);
		    }

		    virtual ~Copy1D(void)
		    {}

		    virtual void Initialize(const size_t szDestinationOffset, const size_t szDestinationSize, const size_t szSourceOffset, const size_t szSourceSize, const size_t szCopySize)
		    {
			    const size_t szData[] = {szDestinationOffset, szDestinationSize, szSourceOffset, szSourceSize, szCopySize};
			    memcpy(m_szInformation, szData, sizeof(size_t)*NbInformations);
		    }

		    virtual void copy(void* pDestination, const void* pSource) const override;
    };
/*
class RTCopy2D : public RTCopy
{
	public:
		struct SBufferPointer2D
		{
			size_t m_Offset[2];
			size_t m_Size[2];

			SBufferPointer2D(const size_t szOffsetX, const size_t szOffsetY, const size_t szSizeX, const size_t szSizeY)
			{
				m_Offset[0] = szOffsetX;
				m_Offset[1] = szOffsetY;
				m_Size[0] = szSizeX;
				m_Size[1] = szSizeY;
			}

			void operator*(const float fFactor)
			{
				m_Offset[0] *= fFactor;
				m_Offset[1] *= fFactor;
				m_Size[0] *= fFactor;
				m_Size[1] *= fFactor;
			}
		};

	protected:
		SBufferPointer2D m_Destination;
		SBufferPointer2D m_Source;
		size_t			 m_Size[2];

	public:
		RTCopy2D()
		{
			memset(m_szInformation, 0, sizeof(size_t)*NbInformations);
		}

		virtual ~RTCopy2D(void)
		{}

		virtual void Initialize(const SBufferPointer2D& destination, const SBufferPointer2D& source, const size_t szWidth, const size_t szHeight, const size_t szPixelByte)
		{
			m_Destination = destination * szPixelByte;
			m_Source = source * szPixelByte
			m_Size[0] = szWidth*szPixelByte;
			m_Size[1] = szHeight*szPixelByte;
		}

		virtual void Copy(void* pDestination, const void* pSource) const;
};
*/
}