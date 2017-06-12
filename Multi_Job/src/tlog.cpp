#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>

#include "tlog.h"

using namespace boss;


int CTLog::MakeDir( const char *sFullName )
{
    char sDir[ 256 ] = {0};
    int iPos = 0;
    int iCnt = 0;
    const char *ptr;

    while ( true ) {
        if ( ( ptr = index( sFullName + iPos, '/' ) ) == 0 )
            break;
        iCnt = ptr - sFullName + 1;
        iPos = iCnt;
        bzero( sDir, sizeof( sDir ) );
        strncpy( sDir, sFullName, iCnt );
        if ( mkdir( sDir, 0777 ) < 0 ) {
            if ( errno != EEXIST )
                return -1;
        }
    }
    return 0;
}

const char * CTLog::GetTime()
{
    struct timeval stNow;
    gettimeofday(&stNow,NULL);
    struct tm p;
    localtime_r(&stNow.tv_sec,&p);
    uint32_t dwTimeLength = strftime( m_szTime, sizeof( m_szTime ),"%Y-%m-%d %H:%M:%S",&p);
    snprintf(m_szTime + dwTimeLength, sizeof(m_szTime) - dwTimeLength, ".%.03f",stNow.tv_usec/(double)1000);
    return m_szTime;
}

int CTLog::ShiftFiles()
{
    //文件特别大时,stat会失败，此时仍然移动
    struct stat stStat;
    int iRet = stat(m_sLogName.c_str(), &stStat);
    if ( iRet < 0 ) {
        this->Log2File(LOG_PRIORITY_ERR,"[%s:%d][%s] ERR: can't stat file:[%s]",
                       basename(__FILE__),__LINE__,__FUNCTION__,strerror(errno));
    }

    if ( iRet == 0 && stStat.st_size < m_iMaxLogSize && stStat.st_size < 1000*1024*1024 ) {
        return 0;
    }

    if ( rename(m_sLogName.c_str(),(m_sLogName + ".tmp").c_str()) < 0 ) {
        this->Log2File(LOG_PRIORITY_ERR,"[%s:%d][%s] ERR: can't rename file:[%s]" 
                       ,basename(__FILE__),__LINE__,__FUNCTION__,strerror(errno));
        return -1;
    }

    this->Log2File(LOG_PRIORITY_NORMAL,"[%s:%d][%s] MSG: move file ok，size=%u, maxsize=%d "
                   ,basename(__FILE__),__LINE__,__FUNCTION__,stStat.st_size,m_iMaxLogSize);

    std::string  sLogBase(m_sLogName.begin(),m_sLogName.end() - 4);
    char szLogFileName[300]={0};
    snprintf(szLogFileName, 300,"%s%d.log", sLogBase.c_str(), m_iMaxLogNum - 1);
    if ( access(szLogFileName,F_OK) == 0 ) {
        if ( remove(szLogFileName) < 0 ) {
            return -1;
        }
    }

    char sNewLogFileName[300] = {0};
    for ( int i = m_iMaxLogNum - 2; i >= 0; --i ) {
        if ( i == 0 ) {
            snprintf(szLogFileName, 300, "%s.log.tmp", sLogBase.c_str());
        } else {
            snprintf(szLogFileName, 300, "%s%d.log", sLogBase.c_str(), i);
        }

        if ( access(szLogFileName, F_OK) == 0 ) {
            snprintf(sNewLogFileName, 300, "%s%d.log", sLogBase.c_str(), i + 1);
            if ( rename(szLogFileName, sNewLogFileName) < 0 ) {
                return -1;
            }
        }
    }
    return 0;
}

int CTLog::Log2File(int iLogLevel,const char *sFormat, ...)
{
    if ( m_iLevel < iLogLevel )return 0;
    va_list ap;
    va_start(ap, sFormat);
    this->LogMsg2File(sFormat,ap);
    va_end(ap);
    return 0;
}

void CTLog::Init(const char * pLogName,int iLogLevel,int iMaxSize,int iMaxNum)
{
    m_iLevel = iLogLevel;
    if ( !pLogName )return;
    m_sLogName = pLogName;
    m_sLogName += ".log";
    m_iMaxLogSize = iMaxSize;
    m_iMaxLogNum  = iMaxNum;
    MakeDir(m_sLogName.c_str());
    m_isReady  = true;
}

bool CTLog::IsClosed()
{
    if ( m_sLogName.empty() )return true;

    struct stat st;
    if ( stat((m_sLogName + "_none").c_str(),&st) == 0 ) {
        return true;
    }
    return false;
}

void CTLog::LogMsg(int iLogLevel,const char *sFormat, ...)
{
    int iErrNo = errno;
    if ( m_isShow ) {
        va_list ap1;
        va_start(ap1, sFormat);
        printf("[%s][pid:%ld]",this->GetTime(),(long)getpid());
        vprintf(sFormat,ap1);
        va_end(ap1);
        printf("\n");
    }

    if ( m_iLevel >= iLogLevel ) {
        va_list ap1;
        va_start(ap1, sFormat);
        LogMsg2File(sFormat,ap1);
        va_end(ap1);
        ShiftFiles();
    }
    if ( m_iOnLineLevel > 0 && m_iOnLineLevel <= iLogLevel ) {

        va_list ap1;
        va_start(ap1, sFormat);
        LogMsgOnline(sFormat,ap1);
        va_end(ap1);
    }
    errno = iErrNo;
}

void CTLog::LogMsgOnline(const char *sFormat,va_list ap)
{
    if ( !m_pOnlineLog || m_dwUsedOnLineSize >= max_online_log_len ) {
        return;
    }
    if(m_dwUsedOnLineSize < max_online_log_len ){
        m_dwUsedOnLineSize += snprintf(m_pOnlineLog + m_dwUsedOnLineSize,max_online_log_len - m_dwUsedOnLineSize,"[%s][pid:%ld]",this->GetTime(),(long)getpid());
    }
    if(m_dwUsedOnLineSize < max_online_log_len ){
        m_dwUsedOnLineSize += vsnprintf(m_pOnlineLog + m_dwUsedOnLineSize,max_online_log_len - m_dwUsedOnLineSize,sFormat,ap);
    }
    if(m_dwUsedOnLineSize < max_online_log_len ){
        m_dwUsedOnLineSize += snprintf(m_pOnlineLog + m_dwUsedOnLineSize,max_online_log_len - m_dwUsedOnLineSize,"\n");
    }
}

void CTLog::LogMsg2File(const char *sFormat, va_list ap)
{
    if ( IsClosed() ) {
        return;
    }

    if ( !m_isReady )return;

    FILE * pstFile = fopen(m_sLogName.c_str(), "a+");
    if ( !pstFile ) {
        return;
    }

    fprintf(pstFile, "[%s][pid:%ld]", this->GetTime(),(long)getpid());
    vfprintf(pstFile, sFormat, ap);
    fprintf(pstFile, "\n");
    fclose(pstFile);
}



