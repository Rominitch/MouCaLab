#include "Dependencies.h"

#include "MouCaCore/include/DatabaseManagerSqlite.h"
#include "MouCaCore/include/DatabaseManager.h"

namespace MouCaCore
{

DatabaseManagerSPtr DatabaseManager::createManager()
{
	return DatabaseManagerSPtr(new DatabaseManagerSqlite());
}

}