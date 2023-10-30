#include "logger.h"


using namespace std::literals;

void MyFormatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm) {
    // выводим атрибут LineID

    boost::json::value out = {
                                {"timestamp", to_iso_extended_string(*rec[timestamp])},
                                {"data", *rec[additional_data]},
                                {"message", *rec[boost::log::expressions::smessage]}
                            };

    strm << out;
} 

void InitBoostLogFilter() {
    boost::log::add_common_attributes();

    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = &MyFormatter,
        boost::log::keywords::auto_flush = true
    );
}

void Logger::StartServerLog(int port, std::string_view address) {

    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, {{"port", port}, {"address", address}}) << "server started"sv;

}

void Logger::StopServerLog(int code, std::string_view what_error) {
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, {{"code", code}, {"exception", what_error}}) << "server exited"sv;
}

void Logger::RequestLog(std::string_view address, std::string_view uri, std::string_view method) {

    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, {{"ip", address}, {"URI", uri}, {"method", method}}) << "request received"sv;

}

void Logger::ResponseLog(int code, std::string_view content, int time) {

    if(content == "") {
        content = "null";
    }

    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, {
            {"response_time", time}, 
            {"code", code},
            {"content_type", content}
            }) << "response sent"sv;

}

void Logger::ErrorServerLog(int code, std::string_view what_error, std::string_view place) {

    BOOST_LOG_TRIVIAL(error) << boost::log::add_value(additional_data, {
        {"code", code},
        {"text", what_error},
        {"where", place}
    }) << "error"sv;

}