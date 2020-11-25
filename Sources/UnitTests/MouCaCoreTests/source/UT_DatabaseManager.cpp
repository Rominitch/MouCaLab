#include <Dependancies.h>

#include <LibCore/include/CoreByteBuffer.h>
#include <LibCore/include/CoreFile.h>

#include <MouCaCore/include/DatabaseManager.h>

void clean(const std::filesystem::path& file)
{
    if (std::filesystem::exists(file))
    {
        ASSERT_TRUE(std::filesystem::remove(file));
    }
}

TEST(DatabaseManager, createManager)
{
    MouCaCore::DatabaseManagerSPtr db = MouCaCore::DatabaseManager::createManager();
    ASSERT_TRUE(db != nullptr);
}

TEST(DatabaseManager, create)
{
    // Test block
    const Core::StringOS invalidDB(MouCaEnvironment::getOutputPath() / L"InvalidFolder" / L"invalid.db");
    
    std::filesystem::path fileInfo(MouCaEnvironment::getOutputPath() / L"testDB.db");
    //fileInfo.normalize();
    const Core::StringOS databaseFile = fileInfo.wstring();

    // Remove old file
    clean(fileInfo);

    Core::File sqlFile(MouCaEnvironment::getInputPath() / L"libraries" / L"myRequest.sql");
    // Create
    {
        MouCaCore::DatabaseManagerSPtr db = MouCaCore::DatabaseManager::createManager();
        ASSERT_TRUE(db->isNull());
        
        ASSERT_NO_THROW(sqlFile.open());

        // Check invalid API
        EXPECT_THROW(db->createDB(invalidDB, sqlFile), Core::Exception);

        // Check valid import
        EXPECT_NO_THROW(db->createDB(databaseFile, sqlFile));
        ASSERT_FALSE(db->isNull());

        ASSERT_NO_THROW(sqlFile.close());

        ASSERT_NO_THROW(db->release());

        db.reset();
    }

    // Open
    {
        MouCaCore::DatabaseManagerSPtr db = MouCaCore::DatabaseManager::createManager();

        ASSERT_NO_THROW(db->open(databaseFile));

        ASSERT_NO_THROW(db->release());
        db.reset();
    }

    // Create a already open
    {
        MouCaCore::DatabaseManagerSPtr db = MouCaCore::DatabaseManager::createManager();

        ASSERT_NO_THROW(sqlFile.open());

        ASSERT_ANY_THROW(db->createDB(databaseFile, sqlFile));

        ASSERT_NO_THROW(sqlFile.close());

        db.reset();
    }
    
    // Clean files
    clean(fileInfo);
}

TEST(DatabaseManager, openMultiDB)
{
    const std::filesystem::path fileData(MouCaEnvironment::getOutputPath() / L"Data.db");
    const std::filesystem::path fileLabel(MouCaEnvironment::getOutputPath() / L"Internationnal.db");

    // Clean files
    clean(fileData);
    clean(fileLabel);

    MouCaCore::DatabaseManagerSPtr dbMouca = MouCaCore::DatabaseManager::createManager();
    MouCaCore::DatabaseManagerSPtr dbLabel = MouCaCore::DatabaseManager::createManager();

    //fileMouca.normalize();
    //fileLabel.normalize();
    const Core::StringOS databaseMouca = fileData.wstring();
    const Core::StringOS databaseLabel = fileLabel.wstring();

    // Create
    {
        // Allocate label
        EXPECT_NO_THROW(dbLabel->createDB(databaseLabel));
        EXPECT_NO_THROW(dbLabel->release());
        dbLabel.reset();

        EXPECT_NO_THROW(dbMouca->createDB(databaseMouca));

        EXPECT_NO_THROW(dbMouca->attachAnotherDB(databaseLabel, u8"Internationnal"));
        
        // Read file and extract query data
        Core::File sqlMouca(MouCaEnvironment::getInputPath() / "libraries" / "creation.sql");
        ASSERT_NO_THROW(sqlMouca.open());
        const Core::String query = sqlMouca.extractUTF8();
        ASSERT_NO_THROW(sqlMouca.close());

        // Launch creation on both db
        EXPECT_NO_THROW(dbMouca->quickQuery(query));

        EXPECT_NO_THROW(dbMouca->release());
    }
    
    dbMouca.reset();

    // Clean files
    clean(fileData);
    clean(fileLabel);
}

TEST(DatabaseManager, openError)
{
    MouCaCore::DatabaseManagerSPtr dbMouca = MouCaCore::DatabaseManager::createManager();

    ASSERT_ANY_THROW( dbMouca->open(MouCaEnvironment::getInputPath() / "invalid.db") );
}

TEST(DatabaseManager, queryError)
{
    Core::Path fileData(MouCaEnvironment::getOutputPath() / "DataQuery.db");
    clean(fileData);

    MouCaCore::DatabaseManagerSPtr dbMouca = MouCaCore::DatabaseManager::createManager();
    EXPECT_NO_THROW(dbMouca->createDB(fileData));

    const Core::String query = u8"SELECT something but request is dummy;";
    EXPECT_ANY_THROW(dbMouca->quickQuery(query));

    EXPECT_NO_THROW(dbMouca->release());

    clean(fileData);
}

TEST(DatabaseManager, query)
{
    std::filesystem::path fileData(MouCaEnvironment::getOutputPath() / "MouCaDungeonQuery.db");

    // Clean files
    clean(fileData);

    
    MouCaCore::DatabaseManagerSPtr dbMouca = MouCaCore::DatabaseManager::createManager();
    // Read file and extract query data
    {
        const Core::StringOS databaseMouca = fileData.wstring();
        Core::File sqlMouca(MouCaEnvironment::getInputPath() / "libraries" / "creation.sql");
        ASSERT_NO_THROW(sqlMouca.open());

        EXPECT_NO_THROW(dbMouca->createDB(databaseMouca, sqlMouca));

        ASSERT_NO_THROW(sqlMouca.close());
    }

    const int64_t      id = 123;
    const Core::String label(u8"MyDEMO");
    const Core::String descr(u8"A Description");
    const double       value = 123.456;
    const uint64_t     blob1 = 1234567ull;
    const float        blob2 = 10.10f;

    // Run query INSERT
    {
        Core::ByteBuffer blob;
        blob << blob1 << blob2;

        const Core::String query = u8"INSERT INTO `GlobalName` VALUES ('"+ std::to_string(id) + u8"', '" + label + u8"', '" + descr + u8"', X'" + blob.exportHex() + u8"', "+ std::to_string(value)+");";
        ASSERT_NO_THROW(dbMouca->quickQuery(query));
    }

    // Run query SELECT
    {
        const Core::String query = u8"SELECT * FROM `GlobalName`;";

        MouCaCore::DatabaseStatementSPtr statement;
        ASSERT_NO_THROW(statement = dbMouca->query(query));
        EXPECT_TRUE(statement.get() != nullptr);
        EXPECT_TRUE(statement->nextRow());

        EXPECT_EQ(5, statement->getNumberColumn());

        EXPECT_EQ(false, statement->isNull(0));
        EXPECT_EQ(true,  statement->readBool(0));
        EXPECT_EQ(id,    statement->readI16(0));
        EXPECT_EQ(id,    statement->readI32(0));
        EXPECT_EQ(id,    statement->readI64(0));
        EXPECT_EQ(label, statement->readString(1));
        EXPECT_EQ(descr, statement->readString(2));
        Core::ByteBuffer blob = statement->readBlob(3);
        
        uint64_t b1;
        float    b2;
        blob >> b1 >> b2;

        EXPECT_EQ(blob1, b1);
        EXPECT_EQ(blob2, b2);

        EXPECT_EQ(value, statement->readDouble(4));
        EXPECT_FALSE(statement->nextRow());

        ASSERT_NO_THROW(statement->release());
    }

    // Run query SELECT valid without result
    {
        const Core::String query = u8"SELECT * FROM `GlobalName` WHERE id_global_name = 1234567;";

        MouCaCore::DatabaseStatementSPtr statement;
        ASSERT_NO_THROW(statement = dbMouca->query(query));
        EXPECT_TRUE(statement.get() != nullptr);
        EXPECT_FALSE(statement->nextRow());

        ASSERT_NO_THROW(statement->release());
    }

    // Run query ERROR
    {
        const Core::String query = u8"SELECT * ABCDEFGHI /* */ FROM `GlobalName`;";

        MouCaCore::DatabaseStatementSPtr statement;
        ASSERT_ANY_THROW(statement = dbMouca->query(query));
        EXPECT_TRUE(statement.get() == nullptr);
    }

    EXPECT_NO_THROW(dbMouca->release());

    clean(fileData);
}