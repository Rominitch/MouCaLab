/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include "LibRT/include/RTCopy.h"

namespace RT
{

void Copy1D::copy(void* pDestination, const void* pSource) const
{
	BT_ASSERT(pDestination != NULL);
	BT_ASSERT(pSource != NULL);

	BT_ASSERT(m_szInformation[DestinationOffset] < m_szInformation[DestinationSize]);
	BT_ASSERT(m_szInformation[SourceOffset] < m_szInformation[SourceSize]);

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

	BT_ASSERT(std::min<size_t>(szDestinationTrueSizeCopy, szSourceTrueSizeCopy) == m_szInformation[CopySize]);
#endif

	char* pDstPtr = &((char*)pDestination)[m_szInformation[DestinationOffset]];
	char* pSrcPtr = &((char*)pSource)[m_szInformation[SourceOffset]];

	memcpy(pDstPtr, pSrcPtr, m_szInformation[CopySize]);
}

}