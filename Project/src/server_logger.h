#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "request_handler.h"

namespace server_logger {

namespace beast = boost::beast;
namespace http = beast::http;

using VariantResponse = std::variant<http_handler::StringResponse, http_handler::FileResponse>;

class LoggingRequestHandler {

    template <typename Body, typename Allocator>
    void LogRequest(http::request<Body, http::basic_fields<Allocator>>&& req, const std::string& address) {    
        Logger::GetInstance().RequestLog(address, req.target(), req.method_string());
        start_time = std::chrono::steady_clock::now();
    };

    void LogResponse(const std::tuple<int, std::string_view>& resp_info);
public:

    LoggingRequestHandler() = delete;
    LoggingRequestHandler(std::shared_ptr<http_handler::RequestHandler>& decorated) : decorated_(decorated) {}

    template <typename Body, typename Allocator, typename Send>
     void operator () (const std::string ip_address, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        LogRequest(std::forward<decltype(req)>(req), ip_address);
        std::tuple<int, std::string_view> resp_info;

        auto log_handler = [&resp_info](VariantResponse&& answer) {

            std::visit([&resp_info](auto&& args) {
                resp_info = std::make_tuple(args.result_int(), args[http::field::content_type]);
            }, answer);

            return std::move(answer);

        };

        (*decorated_)(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send), std::move(log_handler));

        LogResponse(resp_info);
     }

private:
    std::shared_ptr<http_handler::RequestHandler> decorated_;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
};

} // namespace server_logger