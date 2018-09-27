//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_TGENGINE_H
#define DCSS_TGENGINE_H

#include <json.hpp>
#include "IEngine.h"

PRE_DECLARE_PTR(ITGSpi);

/**
 * base class of all trade engine
 *
 * one strategy may connect to several different trade engines at the same time,
 * we use this to wrap te detail
 */
class ITGEngine : public IEngine
{
public:
    ITGEngine();
    ~ITGEngine() override = default;

public:
    std::string Name() { return mName;}

    void SetReaderThread() override ;

protected:
    /*add strategy, return true if added successfully*/
    bool RegisterClient(const std::string& name, const std::string& request);
    /*remove strategy*/
    bool RemoveClient(const std::string& name);

protected:
    void Listening();

protected:

    /*vec of all accounts*/
//    TradeAccount mAccounts;
    /*current nano time*/
    long mCurTime;
    /*-1 if do not accept unregistered account*/
    int mDefaultAccountIndex;

    // tg name
    std::string mName;

private:
    //<client, clientSpi>
    std::unordered_map<std::string, ITGSpiPtr> mSpiMap;
};

#endif //DEMO_TGENGINE_H
