#include "Dependencies.h"

#include "LibCore/include/CoreByteBuffer.h"
#include "LibCore/include/CoreFile.h"
#include "LibCore/include/CoreResource.h"
#include "MouCaCore/include/DatabaseManagerSqlite.h"

namespace MouCaCore
{

DatabaseManagerSqlite::DatabaseManagerSqlite():
_database(nullptr)
{
    MOUCA_PRE_CONDITION(isNull());
}

DatabaseManagerSqlite::~DatabaseManagerSqlite()
{
    MOUCA_PRE_CONDITION(isNull()); // DEV Issue: Need to call release !
}

void DatabaseManagerSqlite::release()
{
    MOUCA_PRE_CONDITION(!isNull()); // DEV Issue: Need to open() or createDB()

    sqlite3_close_v2(_database);
    _database = nullptr;
    _filename.clear();

    MOUCA_PRE_CONDITION(isNull());  // DEV Issue: release doesn't clean properly all initialized object  ?

}

void DatabaseManagerSqlite::configure()
{
    MOUCA_PRE_CONDITION(!isNull());                     // DEV Issue: Need to open() or createDB()

    const Core::String query(u8"PRAGMA foreign_keys = ON;");
    quickQuery(query);
}

void DatabaseManagerSqlite::quickQuery(const Core::String& query)
{
    MOUCA_PRE_CONDITION(!isNull()); // DEV Issue: Need to open() or createDB()
    MOUCA_PRE_CONDITION(!query.empty());

    sqlite3_stmt *ppStmt = nullptr;
    const char*  currentQuery = query.c_str();
    const char*  nextQuery = nullptr;
    do 
    {
        int rc = sqlite3_prepare_v2(_database, currentQuery, -1, &ppStmt, &nextQuery);
        if(rc != SQLITE_OK)
        {
            const Core::String error(sqlite3_errmsg(_database));
            MOUCA_THROW_ERROR_1(u8"DataError", u8"Reading", error);
        }
        // this happens for a comment or white-space
        if(ppStmt == nullptr)
        {
            currentQuery = nextQuery;
            continue;
        }

        // Execute without check
        do 
        {
            rc = sqlite3_step(ppStmt);
        }
        while(rc == SQLITE_ROW);

        rc = sqlite3_finalize(ppStmt);
        if(rc != SQLITE_OK)
        {
// DisableCodeCoverage
            const Core::String error(sqlite3_errmsg(_database));
            MOUCA_THROW_ERROR_1(u8"DataError", u8"EndReading", error);
        }
// EnableCodeCoverage

        currentQuery = nextQuery;
    }
    while(currentQuery != nullptr && currentQuery[0] != '\0');
}

void DatabaseManagerSqlite::open(const Core::StringOS& databaseFile)
{
    MOUCA_PRE_CONDITION(isNull());                     // DEV Issue: Must be release !
    MOUCA_PRE_CONDITION(!databaseFile.empty());        // DEV Issue: Need file !

    _filename = databaseFile;

    //Try to open database
    const Core::String fileUTF8 = Core::convertToU8(_filename);

    //Create file
    if(sqlite3_open_v2(fileUTF8.c_str(), &_database, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK)
    {
        const Core::String error(sqlite3_errmsg(_database));
        release();
        MOUCA_THROW_ERROR_1(u8"DataError", u8"OpenFailed", error);
    }
    MOUCA_ASSERT(!isNull());

    configure();
}

void DatabaseManagerSqlite::createDB(const Core::Path& databaseFile, const Core::File& sqlRequest)
{
    MOUCA_PRE_CONDITION(isNull());                     // DEV Issue: Must be release !
    MOUCA_PRE_CONDITION(!databaseFile.empty());        // DEV Issue: Need file !

    if(Core::File::isExist(databaseFile))
    {
        MOUCA_THROW_ERROR_1(u8"DataError", u8"OpenOverwriteFailed", Core::convertToU8(databaseFile));
    }

    _filename = databaseFile;
    const Core::String fileUTF8 = Core::convertToU8(_filename);

    //Create file
    if(sqlite3_open_v2(fileUTF8.c_str(), &_database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        const Core::String error(sqlite3_errmsg(_database));
        release();
        MOUCA_THROW_ERROR_1(u8"DataError", u8"OpenFailed", error);
    }
    MOUCA_POST_CONDITION(_database != nullptr);

    configure();

    //Apply SQL request (in UTF-8)
    if( sqlRequest.isOpened() )
    {
        MOUCA_ASSERT(sqlRequest.isExist() && sqlRequest.isOpened()); //DEV Issue: Sql request file !

        const Core::String query = sqlRequest.extractUTF8();
        quickQuery(query);
    }
}

void DatabaseManagerSqlite::attachAnotherDB(const Core::Path& filePath, const Core::String& databaseName)
{
    MOUCA_PRE_CONDITION(!isNull());                     // DEV Issue: Need to open() or createDB()
    MOUCA_PRE_CONDITION(std::filesystem::exists(std::filesystem::path(filePath)));

    //Create ATTACH query
    const Core::String query = u8"ATTACH `" + Core::convertToU8(filePath) + u8"` AS `" + databaseName + u8"`;";
    quickQuery(query.c_str());
}

DatabaseStatementSPtr DatabaseManagerSqlite::query(const Core::String& query)
{
    auto statement = std::make_shared<DatabaseStatementSqlite>();

    sqlite3_stmt *ppStmt = nullptr;
    const char* nextStep = nullptr;
    int rc = sqlite3_prepare_v2(_database, query.c_str(), static_cast<int>(query.size()*sizeof(char)), &ppStmt, &nextStep);
    if(rc != SQLITE_OK)
    {
        const Core::String error(sqlite3_errmsg(_database));
        MOUCA_THROW_ERROR_1(u8"DataError", u8"Reading", error);
    }

    MOUCA_ASSERT(nextStep == &query.c_str()[query.size()]); //DEV Issue: unsupported multi query here !

    statement->initialize(ppStmt);

    //Execute first before leave
    //statement->nextRow();

    return statement;
}

DatabaseStatementSqlite::DatabaseStatementSqlite():
_stmt(nullptr)
{
    MOUCA_PRE_CONDITION( _stmt == nullptr );
}

DatabaseStatementSqlite::~DatabaseStatementSqlite()
{
    MOUCA_PRE_CONDITION( _stmt == nullptr ); // DEV Issue: Missing calling release();
}

void DatabaseStatementSqlite::initialize( sqlite3_stmt* stmt )
{
    MOUCA_PRE_CONDITION( _stmt == nullptr );
    MOUCA_PRE_CONDITION( stmt  != nullptr );

    // Copy statement pointer
    _stmt = stmt;
}

void DatabaseStatementSqlite::release()
{
    // Release statement
    const int rc = sqlite3_finalize( _stmt );
    if( rc != SQLITE_OK )
    {
// DisableCodeCoverage
        const Core::String error( sqlite3_errmsg( sqlite3_db_handle(_stmt) ) );
        MOUCA_THROW_ERROR_1( u8"DataError", u8"EndReading", error );
    }
// EnableCodeCoverage
    _stmt = nullptr;
}

int DatabaseStatementSqlite::getNumberColumn() const
{
    MOUCA_PRE_CONDITION( _stmt != nullptr );
    return sqlite3_column_count( _stmt );
}

bool DatabaseStatementSqlite::nextRow() const
{
    MOUCA_PRE_CONDITION( _stmt != nullptr );
    return sqlite3_step( _stmt ) == SQLITE_ROW;
}

bool DatabaseStatementSqlite::isNull(const int columnID) const
{
    MOUCA_PRE_CONDITION(_stmt != nullptr);
    return sqlite3_column_type(_stmt, columnID) == SQLITE_NULL;
}

Core::String DatabaseStatementSqlite::readString( const int columnID ) const
{
    MOUCA_PRE_CONDITION( _stmt != nullptr );
    MOUCA_PRE_CONDITION( columnID < getNumberColumn());
    
    const unsigned char* text = sqlite3_column_text( _stmt, columnID );
    return Core::String(reinterpret_cast< const char* >(text));
}

bool DatabaseStatementSqlite::readBool(const int columnID) const
{
    return readI32( columnID ) != 0 ? true : false;
}

int16_t DatabaseStatementSqlite::readI16( const int columnID ) const
{
    return static_cast<int16_t>(readI32( columnID ));
}

int32_t DatabaseStatementSqlite::readI32(const int columnID) const
{
    MOUCA_PRE_CONDITION(_stmt != nullptr);
    MOUCA_PRE_CONDITION(columnID < getNumberColumn());

    return sqlite3_column_int(_stmt, columnID);
}

int64_t DatabaseStatementSqlite::readI64(const int columnID) const
{
    MOUCA_PRE_CONDITION(_stmt != nullptr);
    MOUCA_PRE_CONDITION(columnID < getNumberColumn());

    return sqlite3_column_int64(_stmt, columnID);
}

double DatabaseStatementSqlite::readDouble(const int columnID) const
{
    MOUCA_PRE_CONDITION(_stmt != nullptr);
    MOUCA_PRE_CONDITION(columnID < getNumberColumn());

    return sqlite3_column_double(_stmt, columnID);
}

float DatabaseStatementSqlite::readFloat( const int columnID ) const
{
    return static_cast<float>(readDouble( columnID ));
}

Core::ByteBuffer DatabaseStatementSqlite::readBlob(const int columnID) const
{
    MOUCA_PRE_CONDITION(_stmt != nullptr);
    MOUCA_PRE_CONDITION(columnID < getNumberColumn());

    int byteCount = sqlite3_column_bytes(_stmt, columnID);
    const void* blob = sqlite3_column_blob(_stmt, columnID);

    Core::ByteBuffer byteBuffer;
    byteBuffer.readBuffer(blob, byteCount);
    return byteBuffer;
}

}