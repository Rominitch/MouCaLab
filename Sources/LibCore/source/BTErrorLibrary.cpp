#include "Dependancies.h"
#include <LibBasicTools/include/BTErrorLibrary.h>

//Memory Leak Linker
#ifdef _DEBUG
	#ifdef DEBUG_NEW
		#undef DEBUG_NEW
	#endif
	#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_NEW
#endif

namespace BT
{

ErrorLibrary::~ErrorLibrary(void)
{
	release();
}

CBTError ErrorLibrary::initialize(const CBTString& strPathLibrary, const CBTString& strLibraryLabel, const CBTString& strCountry)
{
	CBTError pError=NULL;
	if(strPathLibrary.empty()==false && strLibraryLabel.empty()==false)
	{
		
		if(m_strConfigPath.empty()==false)
		{
			CXMLParser Parser;
			pError=Parser.OpenXMLFile(m_strConfigPath);
			if(pError==NULL)
			{
				CXMLResult Result;
				pError = Parser.GetNode(L"Configuration", Result);
				if(pError==NULL && Result.GetNbElements()==1)
				{
					//Go to node
					Parser.PushNode(Result.GetNode(0));

					//Get and control version
					float fVersion=0.0f;
					pError = Result.GetNode(0).GetAttribut(L"version", fVersion);
					if(pError==NULL && m_fVersion==fVersion)
					{
						//Load version 1 info
						if(LoadVersion1(Parser) == NULL)
                        {
                            
                        }
					}

					Parser.PopNode();
				}
				else
				{
				}
			}
			else
			{
			}
		}
		else
		{

		}
	}
	return bInitialize;
}

void ErrorLibrary::release()
{
	std::map<CBTString, ErrorDescription*>::iterator itError = m_mapErrors.begin();
	while(itError != m_mapErrors.end())
	{
		//Release error description
		delete itError->second;
		itError->second=NULL;

		//Next error
		++itError;
	}

	m_mapErrors.clear();
}

ErrorDescription const * const ErrorLibrary::getDescription(const CBTString& strLabel) const
{
	ErrorDescription const * pDescription=NULL;
	std::map<CBTString, ErrorDescription*>::const_iterator itError = m_mapErrors.find(strLabel);
	if(itError != m_mapErrors.end())
	{
		pDescription = itError->second;
	}

	return pDescription;
}

}