#include <stdio.h>
#include <stdlib.h>
#include "errmsg.h"
#include "dbaccess.h"



namespace qm 
{
namespace util
{

///////////////////////interface class ////////////////////////////

class DBOper
{
public:
	DBOper(){}
	virtual ~DBOper(){}
	virtual int Query(const SqlBind & oSqlBind){return 0;}
	virtual uint32_t Insert4AutoID(const SqlBind & oSqlBind){return 0;};
	virtual DBResult & Result()=0;
	virtual void FreeResult(){}
	virtual int Begin(){return 0;}
	virtual int Commit(){return 0;}
	virtual int RollBack(){return 0;}
	virtual int Reconnect(){return 0;}
	virtual std::string GetSql(const SqlBind & oSqlBind);
	virtual std::string Escape(const std::string & sSrc){return sSrc;}
	virtual const char * GetErrMsg(){return "";}
	virtual uint32_t GetErrno(){return 0;}
};


////////////////////////mysql class implementation/////////////////

class ResultMysql:public DBResult
{
public:
	ResultMysql(){this->Init(NULL);}
	virtual ~ResultMysql();
	void Init(MYSQL_RES * pResult);
	virtual int GetInt(){return atoi(m_pCur);}
	virtual uint32_t GetUint(){return strtoul(m_pCur,NULL,10);}
	virtual int64_t GetInt64(){return strtoll(m_pCur,NULL,10);}
	virtual uint64_t GetUint64(){return strtoull(m_pCur,NULL,10);}
	virtual std::string GetString(){return m_pCur;}
	virtual bool Next();
	virtual bool IsEnd(){return m_bEnd;}
	virtual int RowCount(){return m_iRowCount;}
private:
	MYSQL_RES * m_pResult;
	int m_iNumFields;
	int m_iCurField;
	int m_iRowCount;
	MYSQL_ROW m_stRow;
	const char * m_pCur;
	bool m_bEnd;
};

class MysqlOper:public DBOper
{
public:
	MysqlOper(MYSQL * pMysql);
	MysqlOper(const std::string & sIP,int iPort,const std::string & sUser,
		const std::string & sPass,const std::string & sDBName,const std::string & sCharSet);
	virtual ~MysqlOper();
	virtual int Query(const SqlBind & oSqlBind);
	virtual uint32_t Insert4AutoID(const SqlBind & oSqlBind);
	virtual DBResult & Result(){return m_oResultWrapper;}
	virtual void FreeResult();
	virtual int Begin();
	virtual int Commit();
	virtual int RollBack();
	virtual int Reconnect();
	virtual std::string Escape(const std::string & sSrc);
	virtual const char * GetErrMsg(){return m_szErrMsg;}
	virtual uint32_t GetErrno(){return m_iErrno;}
private:
	MYSQL * m_pMysql;
	MYSQL_RES * m_pResult;
	ResultMysql m_oResultWrapper;
	bool m_bAutoCommit;
	bool m_bSelfish;
	char m_szErrMsg[400];
	uint32_t m_iErrno;
};

};//
};//


using namespace std;
using namespace qm::util;

///////////////////////sqlbind implementation//////////////////////
SqlBind & SqlBind::operator << (uint32_t dwIndex)
{
	char szBuff[20] = {0};
	snprintf(szBuff,sizeof(szBuff),"%u",dwIndex);
	sPattern.append(szBuff);
	return *this;
}
SqlBind & SqlBind::operator << (const std::string & s)
{
	sPattern.append(s);
	return *this;
}
void SqlBind::SetInt(uint32_t dwIndex,int32_t iValue)
{
	mpInt.insert(std::map<uint32_t,int32_t>::value_type(dwIndex,iValue));
}
void SqlBind::SetUint(uint32_t dwIndex,uint32_t dwValue)
{
	mpUint.insert(std::map<uint32_t,uint32_t>::value_type(dwIndex,dwValue));
}
void SqlBind::SetInt64(uint32_t dwIndex,int64_t llValue)
{
	char szValue[32] = {0};
	snprintf(szValue,sizeof(szValue),"%ld",llValue);
	mpStr.insert(std::map<uint32_t,std::string>::value_type(dwIndex,szValue));
}
void SqlBind::SetUint64(uint32_t dwIndex,uint64_t ullValue)
{
	char szValue[32] = {0};
	snprintf(szValue,sizeof(szValue),"%lu",ullValue);
	mpStr.insert(std::map<uint32_t,std::string>::value_type(dwIndex,szValue));
}
void SqlBind::SetString(uint32_t dwIndex,const std::string & sValue)
{
	mpStr.insert(std::map<uint32_t,std::string>::value_type(dwIndex,sValue));
}

///////////////////dbaccess method implementation//////////////////

DBAccess::DBAccess(MYSQL * pMysql)
{
	m_iRet = 0;
	memset(m_szErrMsg,0,sizeof(m_szErrMsg));
	m_pDBOper = new MysqlOper(pMysql);
}

DBAccess::DBAccess(DBEngineType iType,const std::string & sIP,int iPort,const std::string & sUser,
		const std::string & sPass,const std::string & sDBName,const std::string & sCharSet)
{
	m_iRet = 0;
	memset(m_szErrMsg,0,sizeof(m_szErrMsg));
	if(iType == DB_MYSQL)
	{
		m_pDBOper = new MysqlOper(sIP,iPort,sUser,sPass,sDBName,sCharSet);
	}
}

DBAccess::~DBAccess()
{
	if(m_pDBOper)
	{
		delete m_pDBOper;
		m_pDBOper = NULL;
	}
}
int DBAccess::Query(const SqlBind & oSqlBind)
{
	m_iRet = m_pDBOper->Query(oSqlBind);
	return m_iRet;
}

uint32_t DBAccess::Insert4AutoID(const SqlBind & oSqlBind)
{
	return m_pDBOper->Insert4AutoID(oSqlBind);
}

int DBAccess::Begin()
{
	m_iRet = m_pDBOper->Begin();
	return m_iRet;
}
int DBAccess::Commit()
{
	m_iRet = m_pDBOper->Commit();
	return m_iRet;
}
int DBAccess::RollBack()
{
	m_iRet = m_pDBOper->RollBack();
	return m_iRet;
}
std::string DBAccess::GetSql(const SqlBind & oSqlBind)
{
	return m_pDBOper->GetSql(oSqlBind);
}
//std::string DBAccess::Escape(const std::string & sSrc)
//{
//	return m_pDBOper->Escape(sSrc);
//}
const char * DBAccess::GetErrMsg()
{
	if(m_iRet == 0)//�޴���
	{
		return "";
	}
	if(m_iRet <= -1000) //�ϲ����
	{
		return m_szErrMsg;
	}
	return m_pDBOper->GetErrMsg();
}

uint32_t DBAccess::GetErrno()
{
	return m_pDBOper->GetErrno();
}

DBResult & DBAccess::Result()
{
	return m_pDBOper->Result();
}
void DBAccess::FreeResult()
{
	m_pDBOper->FreeResult();
}

//////////////////dboper method ////////////////////////////////////

string DBOper::GetSql(const SqlBind & oSqlBind)
{
	string sSql;
	int n = oSqlBind.mpInt.size() + oSqlBind.mpUint.size() + oSqlBind.mpStr.size();
	string::size_type p1 = 0;
	string::size_type p2 = 0;
	for(int i = 1; i <= n; i++)
	{
		char szIndex[20]={0};
		snprintf(szIndex,sizeof(szIndex),":%u",i);
		p2 = oSqlBind.sPattern.find(szIndex,p1);
		if(p2 == string::npos)
		{
			break;
		}
		sSql.append(oSqlBind.sPattern,p1,p2-p1);
		map<uint32_t,int32_t>::const_iterator it1 = oSqlBind.mpInt.find(i);
		if(it1 != oSqlBind.mpInt.end())
		{
			char szTmp[20]={0};
			snprintf(szTmp,sizeof(szTmp),"%d",it1->second);
			sSql.append(szTmp);
			p1 = p2 + strlen(szIndex);
			continue;
		}
		map<uint32_t,uint32_t>::const_iterator it2 = oSqlBind.mpUint.find(i);
		if(it2 != oSqlBind.mpUint.end())
		{
			char szTmp[20]={0};
			snprintf(szTmp,sizeof(szTmp),"%u",it2->second);
			sSql.append(szTmp);
			p1 = p2 + strlen(szIndex);
			continue;
		}
		map<uint32_t,string>::const_iterator it3 = oSqlBind.mpStr.find(i);
		if(it3 != oSqlBind.mpStr.end())
		{
			sSql.append(1,'\'');
			sSql.append(this->Escape(it3->second));
			sSql.append(1,'\'');
			p1 = p2 + strlen(szIndex);
			continue;
		}
		sSql.append(szIndex);
		p1 = p2 + strlen(szIndex);
	}
	
	if(p1 < oSqlBind.sPattern.size())
	{
		sSql.append(oSqlBind.sPattern,p1,oSqlBind.sPattern.size()-p1);
	}
	
	return sSql;
}


///////////////////mysql method implementation///////////////////////


void ResultMysql::Init(MYSQL_RES * pResult)
{
	m_pResult = pResult;
	m_iRowCount = 0;
	m_iCurField = -1;
	m_pCur = "";
	if(pResult == NULL)
	{
		m_bEnd = true;
		return;
	}
	m_iNumFields = mysql_num_fields(m_pResult);
	if(m_iNumFields == 0)
	{
		m_bEnd = true;
		return;
	}
	m_bEnd = false;
	this->Next();
}

ResultMysql::~ResultMysql()
{
}

bool ResultMysql::Next()
{
	if(m_bEnd)
	{
		return false;
	}
	m_iCurField = (m_iCurField + 1)%m_iNumFields;
	if(m_iCurField == 0)
	{
		m_stRow = mysql_fetch_row(m_pResult);
		if(m_stRow == NULL)
		{
			m_bEnd = true;
			return false;
		}
		m_iRowCount ++;
	}
	m_pCur = m_stRow[m_iCurField]==NULL?"":m_stRow[m_iCurField];
	return true;
}



MysqlOper::MysqlOper(MYSQL * pMysql)
{
	m_bAutoCommit = true;
	m_pResult = NULL;
	memset(m_szErrMsg,0,sizeof(m_szErrMsg));
	m_pMysql = pMysql;
	m_bSelfish = false;
	bool bReConnect = true;
	if(m_pMysql != NULL)
	{
		mysql_options(m_pMysql,MYSQL_OPT_RECONNECT,(const char *)&bReConnect);//need to set after connect,old version
	}
}

MysqlOper::MysqlOper(const std::string & sIP,int iPort,const std::string & sUser,
		const std::string & sPass,const std::string & sDBName,const std::string & sCharSet)
{
	m_bAutoCommit = true;
	m_pResult = NULL;
	memset(m_szErrMsg,0,sizeof(m_szErrMsg));
	m_pMysql = new MYSQL();
	m_bSelfish = true;
	mysql_init(m_pMysql);
	mysql_options(m_pMysql,MYSQL_SET_CHARSET_NAME,sCharSet.c_str());
	if(!mysql_real_connect(m_pMysql,sIP.c_str(),sUser.c_str(),sPass.c_str(),sDBName.c_str(),iPort,NULL,0))
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"connect to mysql error:%s",mysql_error(m_pMysql));
		return;
	}
	bool bReConnect = true;
	mysql_options(m_pMysql,MYSQL_OPT_RECONNECT,(const char *)&bReConnect);//need to set after connect,old version
	return;
}

MysqlOper::~MysqlOper()
{
	this->FreeResult();
	if(m_bSelfish && m_pMysql)
	{
		mysql_close(m_pMysql);
		delete m_pMysql;
		m_pMysql = NULL;
	}
	else
	{
		m_pMysql = NULL;
	}
}

int MysqlOper::Query(const SqlBind & oSqlBind)
{
	if(m_pMysql == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"error:no connection struct");
		return -1;
	}
	this->FreeResult();
	string sSql = this->GetSql(oSqlBind);
	int ret = mysql_real_query(m_pMysql,sSql.c_str(),sSql.size());
	if(ret == CR_SERVER_GONE_ERROR || ret == CR_SERVER_LOST || ret == 1/*ʵ��ʹ���У������е�ʱ�����Ӷ�ʧ���ص���1*/)//connection lost
	{
		this->Reconnect();
		if(m_bAutoCommit)//can try again
		{
			ret = mysql_real_query(m_pMysql,sSql.c_str(),sSql.size());
		}
	}
	
	if(ret != 0)
	{
		m_iErrno = mysql_errno(m_pMysql);
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"query failed(%d):%s[sql:%s]",ret,mysql_error(m_pMysql),sSql.c_str());
		return -2;
	}
	m_pResult = mysql_store_result(m_pMysql);
	m_oResultWrapper.Init(m_pResult);
	return mysql_affected_rows(m_pMysql);
}

uint32_t MysqlOper::Insert4AutoID(const SqlBind & oSqlBind)
{
	if(m_pMysql == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"error:no connection struct");
		return 0;
	}
	this->FreeResult();
	string sSql = this->GetSql(oSqlBind);
	int ret = mysql_real_query(m_pMysql,sSql.c_str(),sSql.size());
	if(ret == CR_SERVER_GONE_ERROR || ret == CR_SERVER_LOST || ret == 1/*ʵ��ʹ���У������е�ʱ�����Ӷ�ʧ���ص���1*/)//connection lost
	{
		this->Reconnect();
		if(m_bAutoCommit)//can try again
		{
			ret = mysql_real_query(m_pMysql,sSql.c_str(),sSql.size());
		}
	}
	
	if(ret != 0)
	{
		m_iErrno = mysql_errno(m_pMysql);
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"query failed(%d):%s[sql:%s]",ret,mysql_error(m_pMysql),sSql.c_str());
		return 0;
	}
	return mysql_insert_id(m_pMysql);
}

void MysqlOper::FreeResult()
{
	if(m_pResult != NULL)
	{
		mysql_free_result(m_pResult);
		m_pResult = NULL;
	}
	m_oResultWrapper.Init(NULL);
}

int MysqlOper::Reconnect()
{
	int ret = mysql_ping(m_pMysql);
	if(ret != 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"ping to mysql error(%d):%s",ret,mysql_error(m_pMysql));
		return -1;
	}
	bool bReConnect = true;
	mysql_options(m_pMysql,MYSQL_OPT_RECONNECT,(const char *)&bReConnect);//need to set after connect,old version
	return 0;
}


string MysqlOper::Escape(const string & sSrc)
{
	char * pEscaped = new char[sSrc.size()*2+1];
	memset(pEscaped,0,sSrc.size()*2+1);
	mysql_real_escape_string(m_pMysql,pEscaped,sSrc.c_str(),sSrc.size());
	string sDes = pEscaped;
	delete [] pEscaped;
	return sDes;
}

int MysqlOper::Begin()
{
	int ret = mysql_autocommit(m_pMysql,false);
	if(ret != 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"trans begin failed:%s",mysql_error(m_pMysql));
	}
	else
	{
		m_bAutoCommit = false;
	}
	return ret;
}

int MysqlOper::Commit()
{
	int ret = mysql_commit(m_pMysql);
	if(ret != 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"trans commit failed:%s",mysql_error(m_pMysql));
	}
	mysql_autocommit(m_pMysql,true);
	m_bAutoCommit = true;
	return ret;
}

int MysqlOper::RollBack()
{
	int ret = mysql_rollback(m_pMysql);
	if(ret != 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"trans rollback failed:%s",mysql_error(m_pMysql));
	}
	mysql_autocommit(m_pMysql,true);
	m_bAutoCommit = true;
	return ret;
}
