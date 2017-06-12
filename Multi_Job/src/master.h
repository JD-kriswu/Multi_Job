#ifndef _MASTER_HPP_
#define _MASTER_HPP_

#include <map>
#include "db_config.h"
#include "tlog.h"

class CMaster
{
public :
	CMaster();
	~CMaster();

public :
	int Initialize(const char*);
	int Run();
	int SetRunFlag(int);

public :
	enum
	{
		Reload = 1,
		Exit = 2
	};


private :
	typedef struct
	{
		std::string strConfig;
		int iProcessNumber;
		DatabaseConfig tDatabaseConfig;
	}Config;

private :
	int m_iRunFlag;
	Config m_tConfig;
	std::map<int, int> mapWorkerPid;

private :
	int InitLog();
	int InitConfig(const char*);
	int StartAllWorker();
	int StopAllWorker();
	int CheckWorker();
	int ForkOneWorker(int, int);
};

#endif

