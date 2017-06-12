#ifndef _TLOG_H_
#define _TLOG_H_

#include <string>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>

#include "singleton.h"

/*
 ============日志类=================
 可按优先级打印日志

demo:
int main(int argc,char ** argv)
{
    TLOG->Init(argv[0]);
    TLOG->SetConsoleShow(true);
    for(int i = 0;i<10;++i){
        TLOG_MSG("test cody");
        TLOG_ERR("test cody");
        TLOG_DBG("test cody");
    }
    return 0;
}*/

#ifndef _MT_THREAD_
    #define TLOG                   SINGLETON(boss::CTLog)
#else
    #define TLOG                   TSD_SINGLETON(boss::CTLog)
#endif

#define TLOG_ERR(fmt,args...)       if(TLOG->GetLevel() >= boss::LOG_PRIORITY_ERR    || (TLOG->GetOnlineLevel() > 0 && TLOG->GetOnlineLevel() <= boss::LOG_PRIORITY_ERR)) {TLOG->LogMsg(boss::LOG_PRIORITY_ERR   ,"[%s:%d][%s] ERR: "fmt,basename(const_cast<char *>(__FILE__)),__LINE__,__FUNCTION__,##args);}
#define TLOG_MSG(fmt,args...)       if(TLOG->GetLevel() >= boss::LOG_PRIORITY_NORMAL || (TLOG->GetOnlineLevel() > 0 && TLOG->GetOnlineLevel() <= boss::LOG_PRIORITY_NORMAL)) {TLOG->LogMsg(boss::LOG_PRIORITY_NORMAL,"[%s:%d][%s] MSG: "fmt,basename(const_cast<char *>(__FILE__)),__LINE__,__FUNCTION__,##args);}
#define TLOG_DBG(fmt,args...)       if(TLOG->GetLevel() >= boss::LOG_PRIORITY_DEBUG  || (TLOG->GetOnlineLevel() > 0 && TLOG->GetOnlineLevel() <= boss::LOG_PRIORITY_DEBUG)) {TLOG->LogMsg(boss::LOG_PRIORITY_DEBUG ,"[%s:%d][%s] DBG: "fmt,basename(const_cast<char *>(__FILE__)),__LINE__,__FUNCTION__,##args);}
#define TLOG_ONL(fmt,args...)       if(TLOG->GetLevel() >= boss::LOG_PRIORITY_ONLINE || (TLOG->GetOnlineLevel() > 0 && TLOG->GetOnlineLevel() <= boss::LOG_PRIORITY_ONLINE)) {TLOG->LogMsg(boss::LOG_PRIORITY_ONLINE ,"[%s:%d][%s] ONL: "fmt,basename(const_cast<char *>(__FILE__)),__LINE__,__FUNCTION__,##args);}

namespace boss {

/*
    日志优先级, 打印 <= 所设置的优先级值的日志
*/
    enum __LOG_PRIORITY__ {
        LOG_PRIORITY_NOLOG  = 0,        //无日志
        LOG_PRIORITY_ERR    = 1,        //错误日志
        LOG_PRIORITY_NORMAL = 2,        //正常日志
        LOG_PRIORITY_DEBUG  = 3,        //调试日志，定位问题使用
        LOG_PRIORITY_ONLINE = 4,
    };

    class CTLog {
    public:
        const static int max_online_log_len = 1024*100;

        CTLog():m_iMaxLogSize(10485760),//10M
        m_iMaxLogNum(10),
        m_isReady(false),
        m_isShow(false),//标准输出
        m_iLevel(LOG_PRIORITY_NORMAL),m_iOnLineLevel(0),m_pOnlineLog(NULL),m_dwUsedOnLineSize(0)
        {
            m_szTime[0]='\0';
        };
        /**
         * init tlog
         *
         * @param pLogName   日志文件名
         * @param iLogLevel  see __LOG_PRIORITY__
         */
        void Init(const char * pLogName,int iLogLevel = LOG_PRIORITY_NORMAL,int iMaxSize = 10000000,int iMaxNum = 10);

        void LogMsg(int iLogLevel,const char *sFormat, ...);

        void SetSizeNum(int iMaxSize,int iLogNum)
        {
            m_iMaxLogSize = iMaxSize;
            m_iMaxLogNum = iLogNum;
        }
        void SetConsoleShow(bool isShow)
        {
            m_isShow =  isShow;
        }

        int SetLevel(int iLogLevel)
        {
            int iOld = m_iLevel;
            m_iLevel = iLogLevel;
            return iOld;
        }

        void SetOnLine(int iLogLevel)
        {
            m_iOnLineLevel = iLogLevel;
            m_dwUsedOnLineSize = 0;
            if ( m_iOnLineLevel > 0 && !m_pOnlineLog ) {
                m_pOnlineLog = new char[max_online_log_len];
                m_pOnlineLog[0]= '\0';
                return;
            }

            if ( m_iOnLineLevel == 0 && m_pOnlineLog ) {
                m_pOnlineLog[0]= '\0';
            }
        }

        int GetLevel()
        {
            return m_iLevel;
        }

        int GetOnlineLevel()
        {
            return m_iOnLineLevel;
        }

        ~CTLog()
        {
            if ( m_pOnlineLog ) {
                delete [] m_pOnlineLog;
                m_pOnlineLog = NULL;
            }
        }

        const char * GetOnLine()
        {
            if ( m_pOnlineLog ) {
                return m_pOnlineLog;
            }
            return "";
        }

    private:
        void  LogMsg2File(const char *pFormat, va_list ap);
        void LogMsgOnline(const char *sFormat,va_list ap);
        int ShiftFiles();
        const  char * GetTime();
        int Log2File(int iLogLevel,const char *sFormat, ...);
        int MakeDir( const char *sFullName );

        bool IsClosed();

    private:
        std::string    m_sLogName;
        int     m_iMaxLogSize;
        int     m_iMaxLogNum;
        int     m_isReady ;
        bool    m_isShow;
        int     m_iLevel;
        int     m_iOnLineLevel;
        char    * m_pOnlineLog;
        int     m_dwUsedOnLineSize;
        char    m_szTime[ 64 ];
    };
}
#endif
