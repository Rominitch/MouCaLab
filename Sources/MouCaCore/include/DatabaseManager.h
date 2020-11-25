#pragma once

#include <LibCore/include/CoreResource.h>
#include <LibCore/include/CoreFile.h>

namespace MouCaCore
{
    class DatabaseStatement 
    {
        MOUCA_NOCOPY_NOMOVE(DatabaseStatement);

        public:
            virtual ~DatabaseStatement() = default;

            virtual int getNumberColumn() const = 0;

            virtual bool nextRow() const = 0;

            virtual bool isNull(const int columnID) const = 0;

            virtual Core::String readString(const int columnID) const = 0;
            virtual bool       readBool(const int columnID) const = 0;
            virtual int16_t  readI16(const int columnID) const = 0;
            virtual int32_t  readI32(const int columnID) const = 0;
            virtual int64_t  readI64(const int columnID) const = 0;

            virtual double readDouble(const int columnID) const = 0;
            virtual float readFloat( const int columnID ) const = 0;

            virtual Core::ByteBuffer readBlob(const int columnID) const = 0;

            virtual void release() = 0;

        protected:
            DatabaseStatement() = default;
    };

    using DatabaseStatementSPtr = std::shared_ptr<DatabaseStatement>;

#pragma warning(push)
#pragma warning(disable: 4275)
    class DatabaseManager : public Core::Resource
    {
        MOUCA_NOCOPY_NOMOVE(DatabaseManager);

        public:
            ///	Create standard database player (sqlite).
            ///	\return	Implemented DatabaseManager.
            static std::shared_ptr<DatabaseManager> createManager();

            /// Destructor
            virtual ~DatabaseManager() = default;

            ///	Open existing database.
            ///	\param	filePath: path where read DB.
            ///	\throw	BTException: Can't open DB.
            /// \pre	filePath is not empty
            ///         m_database is empty
            virtual void open(const Core::StringOS& filePath) = 0;

            bool isNull() const override = 0;

            ///	Create database and populate
            ///	\param	filePath: path where write DB.
            ///	\param	sqlRequest: opened file where read SQL request.
            ///	\throw	BTException: Can't create DB.
            ///         BTException: Impossible to execute request.
            /// \pre	filePath is not empty.
            ///         m_database is empty.
            virtual void createDB(const Core::Path& filePath, const Core::File& sqlRequest = Core::File()) = 0;

            virtual void attachAnotherDB(const Core::Path& filePath, const Core::String& databaseName) = 0;
            
            virtual void quickQuery(const Core::String& query) = 0;

            virtual DatabaseStatementSPtr query(const Core::String& query) = 0;

        protected:
            DatabaseManager() = default;
    };
#pragma warning(pop)

    using DatabaseManagerSPtr = std::shared_ptr<DatabaseManager>;
}