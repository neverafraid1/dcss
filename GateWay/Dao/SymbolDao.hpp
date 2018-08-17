//
// Created by wangzhen on 18-7-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SYMBOL_HPP
#define DIGITALCURRENCYSTRATEGYSYSTEM_SYMBOL_HPP

#include "Dao.hpp"
#include <tuple>
#include "Constants.h"

class SymbolDao
{
public:
    static std::vector<std::tuple<std::string, std::string, std::string> > GetAllSymbol(uint8_t source = std::numeric_limits<uint8_t>::max())
    {
        std::vector<std::tuple<std::string, std::string, std::string> > result;
        std::string sql = "select * from symbol";

        switch (source)
        {
        case EXCHANGE_OKCOIN:
        {
            sql.append(" where exchange_type=\"okex\";");
            break;
        }
        case EXCHANGE_BINANCE:
        {
            sql.append(" where exchange_type=\"binance\";");
            break;
        }
        default:
        {
            sql.append(";");
            break;
        }
        }

        try
        {
            auto res = MySqlDao::Instance().ExecuteQuery(sql);
            for (auto& item : res)
            {
                result.emplace_back(std::string(item["symbol"]), std::string(item["base_currency"]), std::string(item["quote_currency"]));
            }
            return std::move(result);
        }
        catch (const std::exception& e)
        {
            return {};
        }
    }
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_SYMBOL_HPP
