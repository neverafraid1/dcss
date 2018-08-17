//
// Created by wangzhen on 18-7-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DAO_HPP
#define DIGITALCURRENCYSTRATEGYSYSTEM_DAO_HPP

#include <mysql++/mysql++.h>
#include <string>
#include <boost/noncopyable.hpp>

class MySqlDao : boost::noncopyable
{
public:

    static MySqlDao& Instance()
    {
        static MySqlDao dao;
        return dao;
    }

    void Init(const std::string& db, const std::string& server, const std::string& user, const std::string& passwd, unsigned int port)
    {
        if (!conn.connect(db.c_str(), server.c_str(), user.c_str(), passwd.c_str(), port))
        {
            std::string str;
            str.append("connect to ").append(server).append(" ").append(db).append(" failed!");
            throw std::runtime_error(str);
        }
    }

    mysqlpp::StoreQueryResult ExecuteQuery(const std::string& sql)
    {
        if (conn.connected())
        {
            auto query = conn.query(sql);
            if (auto res = query.store())
                return res;
            else
            {
                std::string str;
                str.append("execute ").append(sql).append(" failed!");
                throw std::runtime_error(str);
            }
        }
        else
        {
            throw std::runtime_error("db is not connected");
        }
    }

protected:
    MySqlDao() = default;

    virtual ~MySqlDao()
    {
        if (conn.connected())
            conn.disconnect();
    }


private:
    mysqlpp::Connection conn;
};
#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DAO_HPP
