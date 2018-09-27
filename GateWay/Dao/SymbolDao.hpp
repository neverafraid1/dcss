//
// Created by wangzhen on 18-7-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SYMBOL_HPP
#define DIGITALCURRENCYSTRATEGYSYSTEM_SYMBOL_HPP

#include "Dao.hpp"
#include "DataStruct.h"
#include <cstring>

class SymbolDao
{
public:
    static std::vector<DCSSSymbolField> GetAllSymbol(uint8_t source = ExchangeEnum::Min)
    {
        std::vector<DCSSSymbolField> result;
        std::string sql = "select * from symbol";

        switch (source)
        {
        case ExchangeEnum::Okex:
        {
            sql.append(" where exchange_type=\"okex\";");
            break;
        }
        case ExchangeEnum::Binance:
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
                DCSSSymbolField symbol = {};
                symbol.Exchange = static_cast<ExchangeEnum>(source);
                strcpy(symbol.Symbol, item["symbol"].c_str());
                strcpy(symbol.Currency.BaseCurrency, item["base_currency"].c_str());
                strcpy(symbol.Currency.QuoteCurrecy, item["quote_currency"].c_str());
                symbol.PricePrecision = item["price_precision"];
                symbol.AmountPrecision = item["amount_precision"];
                symbol.MinAmount = item["min_amount"];

                result.push_back(symbol);

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
