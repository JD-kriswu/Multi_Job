#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tinystr.h"
#include "tinyxml.h"
#include "master.h"
#include "worker.h"



CMaster::CMaster()
{
	mapWorkerPid.clear();
}

CMaster::~CMaster()
{
	;
}

int CMaster::Initialize(const char* cpszConfig)
{
	if (0 != InitLog())
	{
		printf("InitLog failed");
		return -1;
	}
	if (0 != InitConfig(cpszConfig))
	{
		TLOG_ERR("InitConfig failed");
		return -1;
	}
	
	if (0 != StartAllWorker())
	{
		TLOG_ERR("StartAllWorker failed");
		return -1;
	}


	return 0;
}

int CMaster::Run()
{
	while (1)
	{
		if(Exit == m_iRunFlag)
		{
			if (0 != StopAllWorker())
			{
				TLOG_ERR("StopAllWorker failed");

				return -1;
			}
			else
			{
				TLOG_MSG("StopAllWorker succeeded");
				
				return 0;
			}
		}

		if(Reload == m_iRunFlag)
		{
			if (0 != StopAllWorker())
			{
				TLOG_ERR("StopAllWorker failed");
				return -1;
			}

			if (0 != InitConfig(m_tConfig.strConfig.c_str()))
			{
				TLOG_ERR("InitConfig failed");
				return -1;
			}
		
			if (0 != StartAllWorker())
			{
				TLOG_ERR("StartAllWorker failed");
				return -1;
			}

			TLOG_MSG("reload config succeeded");
			m_iRunFlag = 0;
		}

		CheckWorker();

		usleep(1000);

	}
	return 0;
}

int CMaster::SetRunFlag(int iRunFlag)
{
	m_iRunFlag = iRunFlag;

	return 0;;
}

int CMaster::InitConfig(const char* cpszConfig)
{
	if (NULL == cpszConfig)
	{
		TLOG_ERR("NULL == cpszConfig");
		return -1;
	}
	m_tConfig.strConfig.assign(cpszConfig);

	TiXmlDocument doc(cpszConfig);
	bool bLoadOk = doc.LoadFile();
	if (!bLoadOk)
	{
		TLOG_ERR("please check liveNbaPayBonus.xml");
		return -1;
	}

	TiXmlElement* root = doc.RootElement(); 
	if (NULL == root)
	{
		TLOG_ERR("NULL == root");
		return -1;
	}


	TiXmlNode* tDatabaseNode = root->FirstChild("Database");

	TiXmlNode* tIPNode = tDatabaseNode->FirstChild("IP");
	m_tConfig.tDatabaseConfig.strIP.assign(tIPNode->ToElement()->GetText());

	TiXmlNode* tPortNode = tDatabaseNode->FirstChild("Port");
	m_tConfig.tDatabaseConfig.uiPort = atoi(tPortNode->ToElement()->GetText());

	TiXmlNode* tUserNode = tDatabaseNode->FirstChild("User");
	m_tConfig.tDatabaseConfig.strUser.assign(tUserNode->ToElement()->GetText());

	TiXmlNode* tPasswordNode = tDatabaseNode->FirstChild("Password");
	m_tConfig.tDatabaseConfig.strPassword.assign(tPasswordNode->ToElement()->GetText());

	TiXmlNode* tDBNode = tDatabaseNode->FirstChild("DB");
	m_tConfig.tDatabaseConfig.strDB.assign(tDBNode->ToElement()->GetText());

	TiXmlNode* tCharacterNode = tDatabaseNode->FirstChild("Character");
	m_tConfig.tDatabaseConfig.strCharacter.assign(tCharacterNode->ToElement()->GetText());


	TiXmlNode* tProcessNumberNode = root->FirstChild("ProcessNumber");
	m_tConfig.iProcessNumber = atoi(tProcessNumberNode->ToElement()->GetText());


	return 0;
}

int CMaster::StartAllWorker()
{
	for (int i = 0; i < m_tConfig.iProcessNumber; ++i)
	{
		int iPid = fork();
		if (-1 == iPid)
		{
			TLOG_ERR("fork failed, errno = %d", errno);
			return -1;
		}
		else if (0 == iPid)
		{
			ForkOneWorker(i, iPid);
		}
	}

	return 0;
}

int CMaster::CheckWorker()
{
	for (int i = 0; i < m_tConfig.iProcessNumber; ++i)
	{
		int iRet = waitpid(mapWorkerPid[i], NULL, WNOHANG);
		if (iRet > 0)
		{
			mapWorkerPid[i] = 0;
			int iPid = fork();
			if (-1 == iPid)
			{
				TLOG_ERR("fork failed, errno = %d", errno);
				return -1;
			}
			else if (0 == iPid)
			{
				ForkOneWorker(i, iPid);
			}
		}
	}

	return 0;
}
int CMaster::ForkOneWorker(int iProcessIndex, int iPid)
{
	mapWorkerPid.insert(std::make_pair<int, int>(iProcessIndex, iPid));
	CWorker* tCWorker = new CWorker();
	if (0 != tCWorker->Initialize(m_tConfig.tDatabaseConfig, iProcessIndex, m_tConfig.iProcessNumber))
	{
		mapWorkerPid[iProcessIndex] = 0;
		TLOG_ERR("worker %d Initialize failed", iProcessIndex);
		exit(-1);
	}
	tCWorker->Run();

	delete tCWorker;
	tCWorker = NULL;


	exit(0);
}

int CMaster::InitLog()
{
	char szTmp[256];	
	sprintf(szTmp, "../log/%s", "master");
	TLOG->Init(szTmp, 3);

	return 0;
}

int CMaster::StopAllWorker()
{
 	for (int i = 0; i < m_tConfig.iProcessNumber; ++i)
	{
		kill(mapWorkerPid[i], SIGUSR2);
	}

	return 0;
}


