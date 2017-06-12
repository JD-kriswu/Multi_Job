#include <iostream>
#include <sstream>

#include "dbaccess.h"


using namespace std;


using namespace ibg::util;

struct stDBTest
{
    int id;
    string name;
    int age;
    string address;
    
    stDBTest():id(0),
    name(""),
    age(0),
    address("")
    {}
   
    template <class AR>
    AR& serialize(AR& ar){
        return ar & id & name & age & address;
    }
};


int main()
{
    string ip = "10.6.222.154";
    int  port = 3306;
    string user = "root";
    string password = "root";
    string db_name = "kris_test";
    string charset = "utf8";
    DBAccess  *m_pDBAccess = new DBAccess(DB_MYSQL, ip, port, user, password, db_name, charset);

    stringstream SQL;
    
    SQL << "select id,name,age,address from test001 where name = :1";

    int result = 0;
    SqlBind oSql(SQL.str());

    try{
        oSql.SetString(1,"kriswu");
        std::vector<stDBTest> vecMsg;
        result = m_pDBAccess->Query(oSql, vecMsg);
        if(result < 0){
            cout << "test11111111" << endl;
        }
        if(!vecMsg.empty())
        {
            for(std::vector<stDBTest>::iterator iter = vecMsg.begin();iter != vecMsg.end();iter++)
            {
                cout << iter->name << endl;
            }
        }
    }
    catch(...){
    }
    
    return 0; 
}
