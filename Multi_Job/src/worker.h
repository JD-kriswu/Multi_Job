#ifndef _WORKER_H_
#define _WORKER_H_

#include "db_config.h"
#include "tlog.h"
#include "dbaccess.h"



using namespace qm::util;

class CWorker
{
public :
	CWorker();
	~CWorker();

public :
	int Initialize(const DatabaseConfig&, int, int);
	int Run();

public :
	enum
	{
		Exit = 2,
	};

private :
	int m_iRunFlag;
	int m_iProcessIndex;
	int m_iProcessNumber;
	timeval m_tNow;
	DBAccess *m_pDBAccess;
	DatabaseConfig m_tDatabaseConfig;

private :
	static std::string m_sstrFifo;

private :
	void InitMysql();
	int InitLog();
	int DoTask(timeval);
};

#endif

