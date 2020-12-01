/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include "LibRT/include/RTCopy.h"

namespace RT
{

void Copy1D::copy(void* pDestination, const void* pSource) const
{
	MOUCA_ASSERT(pDestination != NULL);
	MOUCA_ASSERT(pSource != NULL);

	MOUCA_ASSERT(m_szInformation[DestinationOffset] < m_szInformation[DestinationSize]);
	MOUCA_ASSERT(m_szInformation[SourceOffset] < m_szInformation[SourceSize]);

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

	MOUCA_ASSERT(std::min<size_t>(szDestinationTrueSizeCopy, szSourceTrueSizeCopy) == m_szInformation[CopySize]);
#endif

	char* pDstPtr = &((char*)pDestination)[m_szInformation[DestinationOffset]];
	char* pSrcPtr = &((char*)pSource)[m_szInformation[SourceOffset]];

	memcpy(pDstPtr, pSrcPtr, m_szInformation[CopySize]);
}

}