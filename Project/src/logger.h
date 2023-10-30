#pragma once

#include "sdk.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>


#include <boost/date_time.hpp>
#include <boost/json.hpp>


BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

//теперь напишем здесь свой форматтер

void MyFormatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm);

void InitBoostLogFilter();

class Logger {

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    void StartServerLog(int port, std::string_view address);

    void StopServerLog(int code, std::string_view what_error);

    void RequestLog(std::string_view address, std::string_view uri, std::string_view method);

    void ResponseLog(int code, std::string_view content, int time);

    void ErrorServerLog(int code, std::string_view what_error, std::string_view place);

private:

    std::mutex m_;

};