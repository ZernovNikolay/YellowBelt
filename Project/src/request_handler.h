#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "info_holders.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <variant>
#include <sstream>
#include <chrono>

namespace http_handler {

using VariantResponse = std::variant<StringResponse, FileResponse>;

std::string DecodeURI(const std::string& URI);

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:

    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(std::shared_ptr<http_handler::GameDataBaseHolder> database_holder,
                            fs::path&& base_path, Strand api_strand, std::mutex& lock)
        : database_holder_{database_holder}
        , base_path_(std::move(base_path))
        , api_strand_{api_strand}
        , api_request_lock_{lock} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send, typename Log>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send, Log&& log_handler) {

        RequestInfo info_req{req.version(), req.keep_alive(), req.method_string(), req.body(), req.target(), req[http::field::authorization]};

        auto transmitter = [send, log_handler](VariantResponse&& answer) {

            std::visit(
                [&send](auto&& result) {               
                    send(std::forward<decltype(result)>(result));
                },
            log_handler(std::move(answer)));

        };

        std::string URI{info_req.target};
        URI = DecodeURI(URI);
        info_req.target = URI;

        try {
            if (info_req.target.starts_with(TargetType::ADRESS_API)) {

                auto handle = [self = shared_from_this(), send, transmitter,
                               &info_req] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_strand_.running_in_this_thread());
                        transmitter(self->HandleApiRequest(info_req));

                    } catch (...) {
                        transmitter(ErrorHandle(ErrorType::SERVER_ERROR, info_req));
                    }

                };
                return net::dispatch(api_strand_, handle);

                /*{
                    std::lock_guard<std::mutex> lock(api_request_lock_);
                    transmitter(HandleApiRequest(info_req));
                }*/

            }else{

                transmitter(HandleFileRequest(info_req));
            }

        } catch (const RequestException& error) {
            transmitter(ErrorHandle(error.getMessage(), info_req));
        } catch (...) {
            transmitter(ErrorHandle(ErrorType::SERVER_ERROR, info_req));
        }
    }

private:

    StringResponse HandleApiRequest(RequestInfo& info_req) {
        return database_holder_->HandleURI(info_req);
    }

    fs::path CheckIsSubPath(const std::string_view& target);
    void CheckEntryPoint(fs::path& full_path);
    FileResponse CheckDownloadFile(const fs::path& full_path, RequestInfo& info_req);
    FileResponse HandleFileRequest(RequestInfo& info_req);


    std::shared_ptr<http_handler::GameDataBaseHolder> database_holder_;
    fs::path base_path_;
    Strand api_strand_;

    std::mutex& api_request_lock_;
};

} // namespace http_handler

namespace passive_update {

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    // Функция handler будет вызываться внутри strand с интервалом period
    Ticker(std::mutex& api_lock, Strand strand, std::chrono::milliseconds period, Handler handler)
        : api_request_lock_{api_lock}
        , strand_{strand}
        , period_{period}
        , handler_{std::move(handler)} {
    }

    void Start() {
        last_tick_ = Clock::now();
        ScheduleTick();
    }

private:
    void ScheduleTick() {
        timer_.expires_after(period_);
        timer_.async_wait([self = shared_from_this()](boost::system::error_code ec) {
            self->OnTick(ec);
        });
    }

    void OnTick(boost::system::error_code ec) {
        using namespace std::chrono;

        std::lock_guard<std::mutex> lock(api_request_lock_);

        if (!ec) {
            auto this_tick = Clock::now();
            auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
            last_tick_ = this_tick;
            try {
                handler_(delta);
            } catch (...) {
            }
            ScheduleTick();
        }
    }

    using Clock = std::chrono::steady_clock;

    std::mutex& api_request_lock_;
    Strand strand_;
    std::chrono::milliseconds period_;
    boost::asio::steady_timer timer_{strand_};
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};

} // namespace passive_update