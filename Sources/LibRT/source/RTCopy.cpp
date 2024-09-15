/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTCopy.h"

namespace RT
{

void Copy1D::copy(void* pDestination, const void* pSource) const
{
	MouCa::assertion(pDestination != NULL);
	MouCa::assertion(pSource != NULL);

	MouCa::assertion(m_szInformation[DestinationOffset] < m_szInformation[DestinationSize]);
	MouCa::assertion(m_szInformation[SourceOffset] < m_szInformation[SourceSize]);

#ifndef NDEBUG
	size_t szDestinationTrueSizeCopy = m_szInformation[CopySize];
	if(m_szInformation[DestinationOffset]+m_szInformation[CopySize] > m_szInformation[DestinationSize])
	{
		szDestinationTrueSizeCopy = m_szInformation[DestinationSize] - m_szInformation[DestinationOffset];
	}

	size_t szSourceTrueSizeCopy = m_szInformation[CopySize];
	if(m_szInformation[SourceOffset]+m_szInformation[CopySize] > m_szInformation[SourceSize])
	{
		szSourceTrueSizeCopy = m_szInformation[SourceSize] - m_szInformation[SourceOffset];
	}

	MouCa::assertion(std::min<size_t>(szDestinationTrueSizeCopy, szSourceTrueSizeCopy) == m_szInformation[CopySize]);
#endif

	char* pDstPtr = &((char*)pDestination)[m_szInformation[DestinationOffset]];
	char* pSrcPtr = &((char*)pSource)[m_szInformation[SourceOffset]];

	memcpy(pDstPtr, pSrcPtr, m_szInformation[CopySize]);
}

}