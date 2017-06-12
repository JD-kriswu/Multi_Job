#ifndef __DBACCESS_H
#define __DBACCESS_H

#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <time.h>
#include <stdio.h>
#include <mysql.h>

namespace qm {
	namespace util {
		
		class DBOper;
		class DBResult;
		
		enum DBEngineType {
			DB_MYSQL = 0,
		};
		
		class SqlBind {
			public:
				SqlBind(const std::string & sSqlPattern = "") :
						sPattern(sSqlPattern) {
				}
				SqlBind(const char * szSqlPattern) :
						sPattern(szSqlPattern) {
				}
				void SetInt(uint32_t dwIndex, int32_t iValue);
				void SetUint(uint32_t dwIndex, uint32_t dwValue);
				void SetInt64(uint32_t dwIndex, int64_t llValue);
				void SetUint64(uint32_t dwIndex, uint64_t ullValue);
				void SetString(uint32_t dwIndex, const std::string & sValue);
				SqlBind & operator <<(uint32_t dwIndex);
				SqlBind & operator <<(const std::string & s);
			public:
				std::string sPattern;
				std::map<uint32_t, int32_t> mpInt;
				std::map<uint32_t, uint32_t> mpUint;
				std::map<uint32_t, std::string> mpStr;
		};
		
		class DBAccess {
			public:
				DBAccess(MYSQL * pMysql);
				DBAccess(DBEngineType iType, const std::string & sIP, int iPort, const std::string & sUser, const std::string & sPass, const std::string & sDBName, const std::string & sCharSet);
				~DBAccess();
				const char * GetErrMsg();
				uint32_t GetErrno();

				//query,  return rows fetched for select sql; return rows affected for modify sql
				template<class T>
				int Query(const SqlBind & oSqlBind, T & oData);
				int Query(const SqlBind & oSqlBind);
				uint32_t Insert4AutoID(const SqlBind & oSqlBind);

				//for transaction
				int Begin();
				int Commit();
				int RollBack();

				//getsql, for user to print sql
				std::string GetSql(const SqlBind & oSqlBind);

				//std::string Escape(const std::string & s);  
			private:
				DBResult & Result();
				void FreeResult();
			private:
				DBOper * m_pDBOper;
				int m_iRet;
				char m_szErrMsg[100];
		};
		
/////////////////////////////////////////////implementation//////////////////////////////////////////////
		
		class DBResult {
			public:
				DBResult() {
				}
				virtual ~DBResult() {
				}
				virtual int GetInt() {
					return 0;
				}
				virtual uint32_t GetUint() {
					return 0;
				}
				virtual int64_t GetInt64() {
					return 0;
				}
				virtual uint64_t GetUint64() {
					return 0;
				}
				virtual std::string GetString() {
					return "";
				}
				virtual bool Next() {
					return false;
				}
				virtual bool IsEnd() {
					return true;
				}
				virtual int RowCount() {
					return 0;
				}
		};
		
		class DataParser {
			public:
				DataParser(DBResult & refDBResult) :
						m_refDBResult(refDBResult) {
				}
				template<typename T>
				DataParser & operator &(T & d) {
					d.serialize(*this);
					return *this;
				}
				
				inline DataParser & operator &(int32_t & d) {
					if (m_refDBResult.IsEnd()) {
						throw std::string("parser:iterator reach the end.");
					}
					d = m_refDBResult.GetInt();
					m_refDBResult.Next();
					return *this;
				}
				
				inline DataParser & operator &(int64_t & d) {
					if (m_refDBResult.IsEnd()) {
						throw std::string("parser:iterator reach the end.");
					}
					d = m_refDBResult.GetInt64();
					m_refDBResult.Next();
					return *this;
				}
				
				inline DataParser & operator &(std::string & s) {
					if (m_refDBResult.IsEnd()) {
						throw std::string("parser:iterator reach the end.");
					}
					s = m_refDBResult.GetString();
					m_refDBResult.Next();
					return *this;
				}
				inline DataParser & operator &(uint32_t & d) {
					if (m_refDBResult.IsEnd()) {
						throw std::string("parser:iterator reach the end.");
					}
					d = m_refDBResult.GetUint();
					m_refDBResult.Next();
					return *this;
				}
				inline DataParser & operator &(uint64_t & d) {
					if (m_refDBResult.IsEnd()) {
						throw std::string("parser:iterator reach the end.");
					}
					d = m_refDBResult.GetUint64();
					m_refDBResult.Next();
					return *this;
				}
				template<typename T>
				DataParser & operator &(std::list<T> & ls) {
					ls.clear();
					T t;
					while (!m_refDBResult.IsEnd()) {
						(*this) & t;
						ls.push_back(t);
					}
					return *this;
				}
				template<typename T>
				DataParser & operator &(std::vector<T> & v) {
					v.clear();
					T t;
					while (!m_refDBResult.IsEnd()) {
						(*this) & t;
						v.push_back(t);
					}
					return *this;
				}
				template<typename T1, typename T2>
				DataParser & operator &(std::map<T1, T2> & mp) {
					mp.clear();
					T1 t1;
					T2 t2;
					while (!m_refDBResult.IsEnd()) {
						(*this) & t1;
						(*this) & t2;
						mp.insert(typename std::map<T1, T2>::value_type(t1, t2));
					}
					return *this;
				}
			/*	
				inline DataParser & operator &(ibg::serialize::CDummy& o) {
					return *this;
				}
				*/
				template<typename T>
				DataParser & operator >>(T & d) {
					return (*this) & d;
				}
			private:
				DBResult & m_refDBResult;
		};
		
		template<class T>
		int DBAccess::Query(const SqlBind & oSqlBind, T & oData) {
			m_iRet = this->Query(oSqlBind);
			int iRowNum = -1;
			try {
				if (this->Result().IsEnd()) {
					this->FreeResult(); //added 2010.09.20
					return m_iRet;
				}
				DataParser oParser(this->Result());
				oParser >> oData;
				iRowNum = this->Result().RowCount();
			} catch (std::string & s) {
				this->FreeResult();
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", s.c_str());
				m_iRet = -1000;
				return m_iRet;
			}

			this->FreeResult();
			return iRowNum;
		}
	
	}
	;
}
;

#endif
