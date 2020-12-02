#pragma once

#include <MouCaCore/include/Database.h>

namespace MouCaCore
{
    class DatabaseStatementSqlite : public DatabaseStatement
    {
        public:
            DatabaseStatementSqlite();
            ~DatabaseStatementSqlite() override;

            void initialize(sqlite3_stmt* stmt);
            void release() override;

            int getNumberColumn() const override;

            bool nextRow() const override;

            bool isNull(const int columnID) const override;

            Core::String readString( const int columnID ) const override;

            bool readBool(const int columnID) const override;

            //------------------------------------------------------------------------
            /// \brief  Fake API to return int16 (real size in database is 32).
            /// 
            /// \param[in,out] columnID
            /// \returns 
            int16_t readI16(const int columnID) const override;

            int32_t readI32(const int columnID) const override;

            int64_t readI64(const int columnID) const override;

            double readDouble(const int columnID) const override;

            float readFloat( const int columnID ) const override;

            Core::ByteBuffer readBlob(const int columnID) const override;
        private:
            sqlite3_stmt*   _stmt;          ///< Local statement (NEVER delete pointer -> use API !)
    };

    class DatabaseManagerSqlite : public Database
    {
        private:
            sqlite3*        _database;		///< Local database (NEVER delete pointer -> use API !)

            //int callbackQuery(void *NotUsed, int argc, char **argv, char **azColName);

            void configure();

        public:
            /// Constructor
            DatabaseManagerSqlite();

            /// Destructor
            ~DatabaseManagerSqlite() override;

            void release() override;

            bool isNull() const override
            {
                return _filename.empty()
                    && !isLoaded();
            }

            //------------------------------------------------------------------------
            /// \brief  Check if resource is loaded.
            /// 
            /// \returns True if loaded, otherwise false.
            bool isLoaded() const override
            {
                return _database != nullptr;
            }

            ///	Open existing database.
            ///	\param	filePath: path where read DB.
            ///	\throw	BTException: Can't open DB.
            /// \pre	filePath is not empty
            ///         m_database is empty
            void open(const Core::StringOS& filePath) override;

            ///	Create database and populate
            ///	\param	filePath: path where write DB.
            ///	\param	sqlRequest: opened file where read SQL request.
            ///	\throw	BTException: Can't create DB.
            ///         BTException: Impossible to execute request.
            /// \pre	filePath is not empty.
            ///         m_database is empty.
            void createDB(const Core::Path& filePath, const Core::File& sqlRequest = Core::File()) override;

            void attachAnotherDB(const Core::Path& filePath, const Core::String& databaseName) override;

            void quickQuery(const Core::String& query) override;

            //----------------------------------------------------------------------------
            /// \brief Execute a single query with returns.
            ///
            /// \code
            ///     DatabaseStatementSPtr statement = db.query("SELECT * FROM database;");
            ///     while( statement->nextRow() )
            ///     {
            ///         ...
            ///     }
            /// \endcode
            DatabaseStatementSPtr query(const Core::String& query) override;
    };
}