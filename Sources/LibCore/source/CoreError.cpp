/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibCore/include/CoreError.h>

namespace Core
{

Core::String ErrorData::convertMessage(const Core::String& strMessage) const
{
	Core::String strFinalMessage(strMessage);

	for(size_t szParameter=0; szParameter<_parameters.size(); szParameter++)
	{
		//Compute parameter label
		StringStream ssParameter;
		ssParameter<<"%P"<<szParameter;

		size_t szPos=0;
		while((szPos=strFinalMessage.find(ssParameter.str()))!=String::npos)
		{
			//Replace parameter by this value
			strFinalMessage.replace(szPos, ssParameter.str().size(), _parameters[szParameter]);
		}
	}

	//Add \n\r
	size_t szPos=0;
	while((szPos=strFinalMessage.find("\n", szPos))!=String::npos)
	{
		strFinalMessage.insert(szPos, "\r");
		szPos += 2;
	}

	return strFinalMessage;
}

}