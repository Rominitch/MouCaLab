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

void ErrorLibrary::initialize(XML::Parser& parser, const StringView& libraryLabel, const String& country)
{
    MouCa::preCondition(!parser.isLoaded() && !parser.isNull());
    MouCa::preCondition(!libraryLabel.empty());
    MouCa::preCondition(!country.empty());

    //Clear manager
    release();

    //Check data are valid
    if(parser.getFilename().empty() || libraryLabel.empty())
    {
        throw Core::Exception(Core::ErrorData("BasicError", "InvalidPathError") << Core::convertToU8(parser.getFilename()));
    }

    //Create parser and try to open error file
    parser.openXMLFile( parser.getFilename() );
        
    //Read Country part
    XML::NodeUPtr languageNode;
    try
    {
        languageNode = parser.searchNodeView("Language", "country", country);
    }
    catch(...)
    {
        // Get first otherwise
    	languageNode = parser.getNode("Language")->getNode(0);
    }
            
    //Go to node
    try
    {
        const auto aPush = parser.autoPushNode(*languageNode);

        //Read Library part
        XML::NodeUPtr libraryNode = parser.searchNodeView("ErrorLibrary", "name", String(libraryLabel));
        if (libraryNode.get() == nullptr)
        {
            throw Core::Exception(Core::ErrorData("BasicError", "CorruptErrorLibrary"));
        }

        //Go to node
        const auto aPushL = parser.autoPushNode(*libraryNode);

        //Read all errors
        XML::ResultUPtr result = parser.getNode("Error");

        for (size_t errorId = 0; errorId < result->getNbElements(); errorId++)
        {
            const auto aPushE = parser.autoPushNode(*result->getNode(errorId));

            //Get and control version
            Core::String strErrorLabel;
            result->getNode(errorId)->getAttribute("name", strErrorLabel);

            Core::String message, solution;
            //Read all errors
            XML::ResultUPtr messageNode = parser.getNode("Message");
            if (messageNode->getNbElements() != 1)
            {
                throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingNodeError") << "Message");
            }
            messageNode->getNode(0)->getValue(message);

            //Read all errors
            XML::ResultUPtr solutionNode = parser.getNode("Solution");
            if (solutionNode->getNbElements() != 1)
            {
                throw Core::Exception(Core::ErrorData("XMLError", "XMLMissingNodeError") << "Solution");
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
    throw Core::Exception(Core::ErrorData("BasicError", "LibraryErrorUnknown"));
}

}