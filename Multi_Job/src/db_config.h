#ifndef _DB_CONFIG_H_
#define _DB_CONFIG_H_

#include <string>

typedef struct DatabaseConfig
{
	std::string strIP;
	unsigned int uiPort;
	std::string strUser;
	std::string strPassword;
	std::string strDB;
	std::string strCharacter;
	std::string strOrderTablePrefix;
	DatabaseConfig& operator= (const DatabaseConfig& tDatabaseConfig)
	{
		strIP.assign(tDatabaseConfig.strIP);
		uiPort = tDatabaseConfig.uiPort;
		strUser.assign(tDatabaseConfig.strUser);
		strPassword.assign(tDatabaseConfig.strPassword);
		strDB.assign(tDatabaseConfig.strDB);
		strCharacter.assign(tDatabaseConfig.strCharacter);
		strOrderTablePrefix.assign(tDatabaseConfig.strOrderTablePrefix);

		return *this;
	}
}DatabaseConfig;


#endif

