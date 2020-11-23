/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibCore/include/BTError.h>

namespace Core
{

Core::String ErrorData::convertMessage(const Core::String& strMessage) const
{
	Core::String strFinalMessage(strMessage);

	for(size_t szParameter=0; szParameter<_parameters.size(); szParameter++)
	{
		//Compute parameter label
		StringStream ssParameter;
		ssParameter<<u8"%P"<<szParameter;

		size_t szPos=0;
		while((szPos=strFinalMessage.find(ssParameter.str()))!=String::npos)
		{
			//Replace parameter by this value
			strFinalMessage.replace(szPos, ssParameter.str().size(), _parameters[szParameter]);
		}
	}

	//Add \n\r
	size_t szPos=0;
	while((szPos=strFinalMessage.find(u8"\n", szPos))!=String::npos)
	{
		strFinalMessage.insert(szPos, u8"\r");
		szPos += 2;
	}

	return strFinalMessage;
}

}