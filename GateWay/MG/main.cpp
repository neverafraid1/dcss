//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <unordered_set>
#include <csignal>
#include "IMGEngine.h"
#include "Dao.hpp"

std::unordered_set<pid_t> gSonPids;

void SignalHandler(int signum)
{
	std::cout << "father process receive signal: " << signum << std::endl;
	// kill all process in this process group
	for (const auto& item : gSonPids)
		kill(item, SIGTERM);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "please set config file path!" << std::endl;
		return 0;
	}

	MySqlDao::Instance().Init("ccts", "192.168.1.98", "root", "root", 3306);

    std::ifstream in(argv[1]);
    std::ostringstream oss;
    oss << in.rdbuf();
    in.close();

    nlohmann::json config = nlohmann::json::parse(oss.str());
    if (config.count("mgs") == 0 || !config.at("mgs").is_array())
    {
    	std::cerr << "error in parse json. please check config." << std::endl;
    	return 0;
    }
    const auto& mgs = config.at("mgs");

    bool isFather(true);

    pid_t fpid;

    for (const nlohmann::json& item : mgs)
    {
    	if (item.count("name") == 0 || item.count("source") == 0)
    	{
    		std::cerr << "config must contains \"name\" and \"source\"!" << std::endl
    				<< "please check!" << std::endl;

    		for (const auto& item : gSonPids)
    			kill(item, SIGTERM);

    		return 0;
    	}

    	if (isFather)
    	{
    		std::cout << "fork for mg: " << item.at("name") << std::endl;
    		fpid = fork();
    	}

    	if (fpid < 0)
    	{
    		std::cerr << "error in fork!" << std::endl;
    		for (const auto& item : gSonPids)
    			kill(item, SIGTERM);

    		return 0;
    	}

    	if (fpid == 0)
    	{
    		// son thread
    		isFather = false;
    	    std::shared_ptr<IMGEngine> p(new IMGEngine());
    	    p->Load(item);
    	    p->Start();
    	    p->WaitForStop();
    	}
    	else
    		gSonPids.insert(fpid);
    }

    if (isFather)
    {
        std::signal(SIGTERM, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        std::signal(SIGHUP, SignalHandler);
        std::signal(SIGQUIT, SignalHandler);
    	do
    	{
    		std::this_thread::sleep_for(std::chrono::seconds(1));
    	}
    	while (wait(nullptr) != -1);	// avoid orphan process
    }
}
