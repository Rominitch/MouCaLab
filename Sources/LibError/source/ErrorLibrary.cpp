/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependencies.h"

#include <LibError/include/ErrorLibrary.h>

#include <LibXML/include/XML.h>

namespace Core
{

ErrorLibrary::~ErrorLibrary()
{
    release();
}

void ErrorLibrary::initialize(XML::Parser& parser, const Core::String& libraryLabel, const Core::String& country)
{
    MOUCA_PRE_CONDITION(!parser.isLoaded() && !parser.isNull());
    MOUCA_PRE_CONDITION(!libraryLabel.empty());
    MOUCA_PRE_CONDITION(!country.empty());

    //Clear manager
    release();

    //Check data are valid
    if(parser.getFilename().empty() || libraryLabel.empty())
    {
        MOUCA_THROW_ERROR_1(u8"BasicError", u8"InvalidPathError", Core::convertToU8(parser.getFilename()));
    }

    //Create parser and try to open error file
    parser.openXMLFile( parser.getFilename() );
        
    //Read Country part
    XML::NodeUPtr languageNode;
    try
    {
        languageNode = parser.searchNode(u8"Language", u8"country", country);
    }
    catch(...)
    {
        // Get first otherwise
    	languageNode = parser.getNode(u8"Language")->getNode(0);
    }
            
    //Go to node
    try
    {
        const auto aPush = parser.autoPushNode(*languageNode);

        //Read Library part
        XML::NodeUPtr libraryNode = parser.searchNode(u8"ErrorLibrary", u8"name", libraryLabel);
        if (libraryNode.get() == nullptr)
        {
            MOUCA_THROW_ERROR(u8"BasicError", u8"CorruptErrorLibrary");
        }

        //Go to node
        const auto aPushL = parser.autoPushNode(*libraryNode);

        //Read all errors
        XML::ResultUPtr result = parser.getNode(u8"Error");

        for (size_t errorId = 0; errorId < result->getNbElements(); errorId++)
        {
            const auto aPushE = parser.autoPushNode(*result->getNode(errorId));

            //Get and control version
            Core::String strErrorLabel;
            result->getNode(errorId)->getAttribute(u8"name", strErrorLabel);

            Core::String message, solution;
            //Read all errors
            XML::ResultUPtr messageNode = parser.getNode(u8"Message");
            if (messageNode->getNbElements() != 1)
            {
                MOUCA_THROW_ERROR_1(u8"XMLError", u8"XMLMissingNodeError", u8"Message");
            }
            messageNode->getNode(0)->getValue(message);

            //Read all errors
            XML::ResultUPtr solutionNode = parser.getNode(u8"Solution");
            if (solutionNode->getNbElements() != 1)
            {
                MOUCA_THROW_ERROR_1(u8"XMLError", u8"XMLMissingNodeError", u8"Solution");
            }
            solutionNode->getNode(0)->getValue(solution);

            //Add new error
            ErrorDescription* pErrorDescription = new ErrorDescription(message, solution);
            if (pErrorDescription != nullptr)
            {
                _mapErrors[strErrorLabel] = pErrorDescription;
            }
        }
    }
    catch(...)
    {
        parser.release();
        throw;
    }

    parser.release();
}

void ErrorLibrary::release()
{
    auto itError = _mapErrors.begin();
    while(itError != _mapErrors.end())
    {
        //Release error description
        delete itError->second;
        itError->second=NULL;

        //Next error
        ++itError;
    }

    _mapErrors.clear();
}

ErrorDescription const * const ErrorLibrary::getDescription(const Core::String& strLabel) const
{
    const auto itError = _mapErrors.find(strLabel);
    if(itError != _mapErrors.cend())
    {
        return itError->second;
    }
    MOUCA_THROW_ERROR(u8"BasicError", u8"LibraryErrorUnknown");
}

}