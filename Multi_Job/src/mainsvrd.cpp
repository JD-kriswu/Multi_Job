/*
	created by kriswu
*/


#include <sys/resource.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include "tlog.h"
#include "master.h"


int giExitFlag = 0;


int InitDaemon();
void sigusr1_handle(int);
void sigusr2_handle(int);
void GetNameFromPath(const char*, char*);



CMaster* gptCMaster;




int main(int argc, char** argv)
{	
	char szProcName[256];
	GetNameFromPath(argv[0], szProcName);

	char szTmp[256];	
	sprintf(szTmp, "../log/%s", szProcName);
	TLOG->Init(szTmp, 3);

	if(argc < 2)
	{
		TLOG_ERR("usage : %s  conf_file [-d]", argv[0]);
		exit(0);
	}

	int lock_fd = open(argv[1], O_RDWR | O_CREAT, 0640);
	if(lock_fd < 0 )
	{
		TLOG_ERR("open failed, errno = %d", errno);
		return -1;
	}

	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if(ret < 0 )
	{
		TLOG_ERR("flock failed, errno = %d", errno);
		return -1;
	}

	if((argc >= 3) && (0 == strcasecmp(argv[2], "-d")))
	{
		;
	}
	else
	{
		InitDaemon();
	}

	signal(SIGUSR1, sigusr1_handle);
	signal(SIGUSR2, sigusr2_handle);	


	TLOG_MSG("%s begin Start.", argv[0]);
	gptCMaster = new CMaster();

	ret = gptCMaster->Initialize(argv[1]);
	if( 0 != ret )
	{
		TLOG_ERR("gptCMaster->Initialize failed");
		return -1;
	}

	TLOG_MSG("%s Started.", argv[0]);
	gptCMaster->Run();



	return 0;
}


void sigusr1_handle(int iSigVal)
{
	gptCMaster->SetRunFlag(CMaster::Reload);
	signal(SIGUSR1, sigusr1_handle);
}

void sigusr2_handle(int iSigVal)
{
	gptCMaster->SetRunFlag(CMaster::Exit);
	signal(SIGUSR2, sigusr2_handle);
}

int InitDaemon()
{
	pid_t pid;

	if ((pid = fork() ) != 0 )
	{
		exit( 0);
	}

	setsid();

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	sigaction(SIGHUP, &sig, NULL);

	if ((pid = fork()) != 0 )
	{
		exit( 0);
	}

	umask(0);
	setpgrp();
	return 0;
}

void GetNameFromPath(const char* szPath, char* szName) 
{
	int iLen = strlen(szPath);
	if (iLen <= 0)
	{
		szName[0] = 0;
		return; 
	}

	const char* p = szPath + iLen - 1;
	while((*p != '/') && (p != szPath))
	{
		--p;    
	}
	if (p != szPath) 
	{
		strcpy(szName, p + 1); 
	}
	else    
	{
		strcpy(szName, szPath);
	}
	return; 
};
