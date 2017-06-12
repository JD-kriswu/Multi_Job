#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include "worker.h"
#include "test_struct.h"
extern int giExitFlag;


using namespace qm::util;

CWorker::CWorker()
{
	m_iProcessIndex = 0;
	m_iRunFlag = 0;
}

CWorker::~CWorker()
{
        if(m_pDBAccess != NULL)
        {
                delete m_pDBAccess;
                m_pDBAccess = NULL;
        }
}

static void SignalHandler(int sig) 
{
	giExitFlag = 1;
}

int CWorker::Initialize(const DatabaseConfig& tDatabaseConfig, int iProcessIndex, int iProcessNumber)
{
	m_tDatabaseConfig = tDatabaseConfig;
	m_iProcessIndex = iProcessIndex;
	m_iProcessNumber = iProcessNumber;

	if (0 != InitLog())
	{
		printf("InitLog failed\n");
		return -1;
	}

	//init mysql
	InitMysql();


	signal(SIGUSR2, SignalHandler);


	return 0;
}

int CWorker::Run()
{
	while (1)
	{
		gettimeofday(&m_tNow, NULL); 

		static time_t sTimeLast = 0;

		if (m_tNow.tv_sec - sTimeLast > 5)
		{
			sTimeLast = m_tNow.tv_sec;
			if(1 == giExitFlag)
			{
				TLOG_ERR("worker %d exited", m_iProcessIndex);
				return 0;
			}
		}

		usleep(1000);



		DoTask(m_tNow);
	}

	return 0;
}

void CWorker::InitMysql()
{
	m_pDBAccess = new DBAccess(DB_MYSQL, 
								m_tDatabaseConfig.strIP.c_str(), 
								m_tDatabaseConfig.uiPort, 
								m_tDatabaseConfig.strUser.c_str(), 
								m_tDatabaseConfig.strPassword.c_str(), 
								m_tDatabaseConfig.strDB.c_str(), 
								m_tDatabaseConfig.strCharacter.c_str());
}

int CWorker::InitLog()
{
	char szTmp[256];	
	sprintf(szTmp, "../log/%s_%d", "worker", m_iProcessIndex);
	TLOG->Init(szTmp, 3);
	return 0;
}

int CWorker::DoTask(timeval tNow)
{
	static time_t sTimeLast = 0;
	if (tNow.tv_sec - sTimeLast > 30 * 60)
	{
		sTimeLast = tNow.tv_sec;
		stringstream  SQL;
		SQL << "select ruleId ,statType from cMonitorPolicy limit 1";

		int iRet = 0;
		SqlBind oSql(SQL.str());
		dao_test  oDaoTest;

		try {
			iRet = m_pDBAccess -> Query(oSql,oDaoTest);
			if(iRet < 0)
			{
				TLOG_ERR("query db failed msg:%s",m_pDBAccess->GetErrMsg());
				return -1;
			}
		}catch(...){
			TLOG_ERR("query db failed msg:%s",m_pDBAccess->GetErrMsg());
			return -2;
		}
		
		TLOG_MSG("result: ruleId %d  statType  %s",oDaoTest.ruleId,oDaoTest.statType.c_str());	


		TLOG_MSG("worker %d doing", m_iProcessIndex);
	}

	return 0;
}
