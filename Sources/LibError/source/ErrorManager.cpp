/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#include "Dependancies.h"

#include <LibXML/include/XML.h>

#include <LibCore/include/CoreLocale.h>

#include <LibError/include/ErrorPrinter.h>
#include <LibError/include/ErrorLibrary.h>
#include <LibError/include/ErrorManager.h>

namespace Core
{

ErrorManager::ErrorManager()
{}

ErrorManager::~ErrorManager()
{
    release();
}

void ErrorManager::release()
{
    //Release libraries
    auto itLibrary = _mapErrorsLibrary.begin();
    while(itLibrary!=_mapErrorsLibrary.end())
    {
        delete itLibrary->second;
        itLibrary->second=NULL;

        ++itLibrary;
    }
    _mapErrorsLibrary.clear();

    _linkLocale.reset(); // Lost link (but don't delete memory)
}

void ErrorManager::addErrorLibrary(XML::Parser& parser, const String& strLibraryName)
{
    MOUCA_PRE_CONDITION(!parser.isLoaded());
    MOUCA_PRE_CONDITION(!strLibraryName.empty());

    if(parser.getFilename().empty())
    {
        BT_THROW_ERROR_1("BasicError", "InvalidPathError", Core::convertToU8(parser.getFilename()))
    }

    //Search if library already exist
    if(_mapErrorsLibrary.find(strLibraryName)==_mapErrorsLibrary.end())
    {
        ErrorLibrary* pLibrary = new ErrorLibrary();
        if(pLibrary==nullptr)
        {
            BT_THROW_ERROR_1("BasicError", "NULLPointerError", "pLibrary")
        }

        //Try to load library
        try
        {
            const String& country = _linkLocale.expired() ? Locale::defaultCountry : _linkLocale.lock()->getCountry();
            pLibrary->initialize(parser, strLibraryName, country);

            //Copy to manager
            _mapErrorsLibrary[strLibraryName] = pLibrary;
        }
        catch(...)
        {
            //Release object
            delete pLibrary;
                
            //Send error
            throw;
        }	
    }
    else
    {
        BT_THROW_ERROR("BasicError", "LibraryDoubleAddError")
    }
}

void ErrorManager::show(const Exception& Exception) const
{
    MOUCA_PRE_CONDITION(_printer != nullptr); //never happened

#ifndef NDEBUG
    size_t iShow=Exception.getNbErrors()-1;
#else
    size_t iShow=0;
#endif
    const ErrorData& error(Exception.read(iShow));
        
    //Get library (is unique in array)
    ErrorLibrary* pLibrary=nullptr;
    const auto it = _mapErrorsLibrary.find(error.getLibraryLabel());
    if(it != _mapErrorsLibrary.cend())
    {
        pLibrary = it->second;
    }

    //Print message
    _printer->print(error, pLibrary);
}

String ErrorManager::getError(const ErrorData& error) const
{
    StringStream ssStream;

    //Get library (is unique in array)
    ErrorLibrary* pLibrary = nullptr;
    const auto it = _mapErrorsLibrary.find(error.getLibraryLabel());
    if (it != _mapErrorsLibrary.cend())
    {
        pLibrary = it->second;
        
        MOUCA_ASSERT(pLibrary != nullptr);
        const ErrorDescription* pDescription = pLibrary->getDescription(error.getErrorLabel());
        ssStream << error.convertMessage(pDescription->getMessage()) << u8"\r\n" << error.convertMessage(pDescription->getSolution());
    }
    else
    {
        ssStream << error.getLibraryLabel() << u8" " << error.getErrorLabel();
    }
    return ssStream.str();
}

}