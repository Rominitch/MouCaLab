#include <Dependencies.h>

#include <LibCore/include/CoreByteBuffer.h>
#include <LibCore/include/CoreFile.h>

#include <MouCaCore/include/CoreSystem.h>
#include <MouCaCore/include/Database.h>

void clean(const std::filesystem::path& file)
{
    if (std::filesystem::exists(file))
    {
        ASSERT_TRUE(std::filesystem::remove(file));
    }
}

class DatabaseTest : public ::testing::Test
{
    public:
        MouCaCore::DatabaseSPtr createDatabase()
        {
            return _core.getResourceManager().createDatabase();
        }

        void releaseDatabase(MouCaCore::DatabaseSPtr& db)
        {
            _core.getResourceManager().releaseResource(std::move(db));
        }

    protected:
        MouCaCore::CoreSystem _core;
};

TEST_F(DatabaseTest, createManager)
{
    MouCaCore::DatabaseSPtr db = createDatabase();
    ASSERT_TRUE(db != nullptr);
    releaseDatabase(db);
}

TEST_F(DatabaseTest, create)
{
    // Test block
    const Core::StringOS invalidDB(MouCaEnvironment::getOutputPath() / L"InvalidFolder" / L"invalid.db");
    
    const Core::Path fileInfo(MouCaEnvironment::getOutputPath() / L"testDB.db");
    //fileInfo.normalize();
    const Core::StringOS databaseFile = fileInfo.wstring();

    // Remove old file
    clean(fileInfo);

    Core::File sqlFile(MouCaEnvironment::getInputPath() / L"libraries" / L"myRequest.sql");
    // Create
    {
        MouCaCore::DatabaseSPtr db = createDatabase();
        ASSERT_TRUE(db->isNull());
        
        ASSERT_NO_THROW(sqlFile.open());

        // Check invalid API
        EXPECT_THROW(db->createDB(invalidDB, sqlFile), Core::Exception);

        // Check valid import
        EXPECT_NO_THROW(db->createDB(databaseFile, sqlFile));
        ASSERT_FALSE(db->isNull());

        ASSERT_NO_THROW(sqlFile.close());

        ASSERT_NO_THROW(releaseDatabase(db));
    }

    // Open
    {
        MouCaCore::DatabaseSPtr db = createDatabase();

        ASSERT_NO_THROW(db->open(databaseFile));

        ASSERT_NO_THROW(releaseDatabase(db));
    }

    // Create a already open
    {
        MouCaCore::DatabaseSPtr db = createDatabase();

        ASSERT_NO_THROW(sqlFile.open());

        ASSERT_ANY_THROW(db->createDB(databaseFile, sqlFile));

        ASSERT_NO_THROW(sqlFile.close());

        ASSERT_NO_THROW(releaseDatabase(db));
    }
    
    // Clean files
    clean(fileInfo);
}

TEST_F(DatabaseTest, openMultiDB)
{
    const Core::Path fileData(MouCaEnvironment::getOutputPath() / L"Data.db");
    const Core::Path fileLabel(MouCaEnvironment::getOutputPath() / L"Internationnal.db");

    // Clean files
    clean(fileData);
    clean(fileLabel);

    MouCaCore::DatabaseSPtr dbMouca = createDatabase();
    MouCaCore::DatabaseSPtr dbLabel = createDatabase();

    //fileMouca.normalize();
    //fileLabel.normalize();
    const Core::StringOS databaseMouca = fileData.wstring();
    const Core::StringOS databaseLabel = fileLabel.wstring();

    // Create
    {
        // Allocate label
        EXPECT_NO_THROW(dbLabel->createDB(databaseLabel));
        EXPECT_NO_THROW(releaseDatabase(dbLabel));

        EXPECT_NO_THROW(dbMouca->createDB(databaseMouca));

        EXPECT_NO_THROW(dbMouca->attachAnotherDB(databaseLabel, "Internationnal"));
        
        // Read file and extract query data
        Core::File sqlMouca(MouCaEnvironment::getInputPath() / "libraries" / "creation.sql");
        ASSERT_NO_THROW(sqlMouca.open());
        const auto query = sqlMouca.extractUTF8();
        ASSERT_NO_THROW(sqlMouca.close());

        // Launch creation on both db
        EXPECT_NO_THROW(dbMouca->quickQuery(Core::String(query.begin(), query.end())));

        EXPECT_NO_THROW(releaseDatabase(dbMouca));
    }
    
    // Clean files
    clean(fileData);
    clean(fileLabel);
}

TEST_F(DatabaseTest, openError)
{
    MouCaCore::DatabaseSPtr dbMouca = createDatabase();

    ASSERT_ANY_THROW( dbMouca->open(MouCaEnvironment::getInputPath() / "invalid.db") );

    EXPECT_NO_THROW(releaseDatabase(dbMouca));
}

TEST_F(DatabaseTest, queryError)
{
    Core::Path fileData(MouCaEnvironment::getOutputPath() / "DataQuery.db");
    clean(fileData);

    MouCaCore::DatabaseSPtr dbMouca = createDatabase();
    EXPECT_NO_THROW(dbMouca->createDB(fileData));

    const Core::String query = "SELECT something but request is dummy;";
    EXPECT_ANY_THROW(dbMouca->quickQuery(query));

    EXPECT_NO_THROW(releaseDatabase(dbMouca));

    clean(fileData);
}

TEST_F(DatabaseTest, query)
{
    const Core::Path fileData(MouCaEnvironment::getOutputPath() / "MouCaDungeonQuery.db");

    // Clean files
    clean(fileData);

    
    MouCaCore::DatabaseSPtr dbMouca = createDatabase();
    // Read file and extract query data
    {
        const Core::StringOS databaseMouca = fileData.wstring();
        Core::File sqlMouca(MouCaEnvironment::getInputPath() / "libraries" / "creation.sql");
        ASSERT_NO_THROW(sqlMouca.open());

        EXPECT_NO_THROW(dbMouca->createDB(databaseMouca, sqlMouca));

        ASSERT_NO_THROW(sqlMouca.close());
    }

    const int64_t      id = 123;
    const Core::String label("MyDEMO");
    const Core::String descr("A Description");
    const double       value = 123.456;
    const uint64_t     blob1 = 1234567ull;
    const float        blob2 = 10.10f;

    // Run query INSERT
    {
        Core::ByteBuffer blob;
        blob << blob1 << blob2;

        const Core::String query = std::format("INSERT INTO `GlobalName` VALUES ('{}', '{}', '{}', X'{}', {});", std::to_string(id), label, descr, blob.exportHex(), std::to_string(value));
        ASSERT_NO_THROW(dbMouca->quickQuery(query));
    }

    // Run query SELECT
    {
        const Core::String query = "SELECT * FROM `GlobalName`;";

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
        const Core::String query = "SELECT * FROM `GlobalName` WHERE id_global_name = 1234567;";

        MouCaCore::DatabaseStatementSPtr statement;
        ASSERT_NO_THROW(statement = dbMouca->query(query));
        EXPECT_TRUE(statement.get() != nullptr);
        EXPECT_FALSE(statement->nextRow());

        ASSERT_NO_THROW(statement->release());
    }

    // Run query ERROR
    {
        const Core::String query = "SELECT * ABCDEFGHI /* */ FROM `GlobalName`;";

        MouCaCore::DatabaseStatementSPtr statement;
        ASSERT_ANY_THROW(statement = dbMouca->query(query));
        EXPECT_TRUE(statement.get() == nullptr);
    }

    EXPECT_NO_THROW(releaseDatabase(dbMouca));

    clean(fileData);
}