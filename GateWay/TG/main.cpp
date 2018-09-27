//
// Created by wangzhen on 18-6-7.
//

#include <unistd.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "ITGEngine.h"
#include "Dao.hpp"
#include "json.hpp"

int main(int argc, char* argv[])
{
    std::ifstream ifs("/opt/dcss/master/etc/mysql/mysql.json");
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    nlohmann::json config = nlohmann::json::parse(str);

	MySqlDao::Instance().Init(config.at("db"), config.at("host"), config.at("user"), config.at("passwd"), config.at("port"));
    std::shared_ptr<ITGEngine> p(new ITGEngine());
    p->Start();
    p->WaitForStop();
}
