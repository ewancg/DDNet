#ifndef ENGINE_SERVER_DATABASES_CONNECTION_POOL_H
#define ENGINE_SERVER_DATABASES_CONNECTION_POOL_H

#include "connection.h"

#include <base/tl/threading.h>
#include <atomic>
#include <memory>
#include <vector>

struct ISqlData
{
	virtual ~ISqlData() {};
};

class CDbConnectionPool
{
public:
	CDbConnectionPool();
	~CDbConnectionPool();
	CDbConnectionPool& operator=(const CDbConnectionPool&) = delete;

	typedef bool (*FRead)(IDbConnection *, const ISqlData *);
	typedef bool (*FWrite)(IDbConnection *, const ISqlData *, bool);

	enum Mode
	{
		READ,
		WRITE,
		WRITE_BACKUP,
		NUM_MODES,
	};

	void RegisterDatabase(std::unique_ptr<IDbConnection> pDatabase, Mode DatabaseMode);

	void Execute(
			FRead pFunc,
			std::unique_ptr<const ISqlData> pSqlRequestData,
			const char *pName);
	// writes to WRITE_BACKUP server in case of failure
	void ExecuteWrite(
			FWrite pFunc,
			std::unique_ptr<const ISqlData> pSqlRequestData,
			const char *pName);

	void OnShutdown();

private:
	std::vector<std::unique_ptr<IDbConnection>> m_aapDbConnections[NUM_MODES];

	static void Worker(void *pUser);
	void Worker();
	bool ExecSqlFunc(IDbConnection *pConnection, struct CSqlExecData *pData, bool Failure);

	semaphore m_NumElem;
	int FirstElem;
	int LastElem;
	std::unique_ptr<struct CSqlExecData> m_aTasks[512];
};

#endif // ENGINE_SERVER_DATABASES_CONNECTION_POOL_H
