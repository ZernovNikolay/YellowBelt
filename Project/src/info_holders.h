#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include "http_server.h"
#include "http_request_constant.h"
#include "player.h"
#include <experimental/random>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

struct RequestInfo {

    const unsigned http_version;
    const bool keep_alive;
    const std::string_view method;
    const std::string body;
    std::string_view target;
    const std::string_view authorize;
    std::string_view method_allowed;

    RequestInfo(unsigned http_version_, bool keep_alive_, std::string_view method_, std::string body_, std::string_view target_, std::string_view authorize_ = ""sv) 
        : http_version(http_version_)
        , keep_alive(keep_alive_)
        , method(method_)
        , body(body_)
        , target(target_)
        , authorize(authorize_) {}

    RequestInfo(const RequestInfo &) = delete;
    void operator=(const RequestInfo &) = delete;

};

class RequestException {
    public: 
        RequestException(ErrorType error_): error{error_}{}
        ErrorType getMessage() const {return error;}
    private:
        ErrorType error;
};

void CheckMethod(const std::string_view allowed, const std::string_view target);
char CheckDirection(const std::string& body);
int CheckTimeTick(const std::string& body);

FileResponse MakeFileResponse(http::status status, http::file_body::value_type file_body, 
                              RequestInfo& info_req, std::string_view content_type);


StringResponse MakeStringResponse(http::status status, std::string_view body,
                                  RequestInfo& info_req, std::string_view content_type);

StringResponse ErrorHandle(ErrorType error, RequestInfo& info_req);

class GameDataBaseHolder {

typedef StringResponse (GameDataBaseHolder::*method_api)(RequestInfo&);
typedef std::unordered_map<std::string_view, method_api> methods_map;

public:

    struct AuthorizeInfo{

        model::Map::Id map_id;
        std::string new_name;

        AuthorizeInfo(model::Map::Id new_map_id, std::string new_new_name) : map_id(new_map_id), new_name(new_new_name) {}

    };

    GameDataBaseHolder(model::Game&& new_game_,
                    std::shared_ptr<game_unit::PlayerLibrary> players, std::shared_ptr<game_unit::Tokenizer> tokenizer)
     : game_(new_game_)
     , players_(players)
     , tokenizer_(tokenizer) {}


    StringResponse HandleURI(RequestInfo& info_req);

    void UpdateOnTick(std::chrono::milliseconds delta);

    void SetStartParameters(bool time_, bool randomize_) {
        active_time_management = !time_;
        randomize_start_point = randomize_;
    }

private:

    [[nodiscard]] std::shared_ptr<game_unit::Player> Authorization_Process(const std::string_view& authorize_field);
    [[nodiscard]] StringResponse Game_Request(Request_API request, RequestInfo& info_req);
    [[nodiscard]] StringResponse Authorize_Request(RequestInfo& info_req);
    [[nodiscard]] StringResponse PlayerList_Request(RequestInfo& info_req);
    [[nodiscard]] StringResponse GameState_Request(RequestInfo& info_req);
    [[nodiscard]] StringResponse Move_Request(RequestInfo& info_req);
    [[nodiscard]] StringResponse UpdateOnTime_Request(RequestInfo& info_req);
    [[nodiscard]] std::shared_ptr<game_unit::Player> CreateNewPlayer(AuthorizeInfo info);

    model::Game game_;
    std::shared_ptr<game_unit::PlayerLibrary> players_;
    std::shared_ptr<game_unit::Tokenizer> tokenizer_;
    bool active_time_management = true;
    bool randomize_start_point = false;

    const methods_map methods_library = {  {TargetType::ADRESS_JOIN_POINT, &http_handler::GameDataBaseHolder::Authorize_Request}, 
                                            {TargetType::ADRESS_PLAYER_LIST, &http_handler::GameDataBaseHolder::PlayerList_Request},
                                            {TargetType::ADRESS_GAME_STATE, &http_handler::GameDataBaseHolder::GameState_Request},
                                            {TargetType::ADRESS_MOVE_PLAYER, &http_handler::GameDataBaseHolder::Move_Request},
                                            {TargetType::UPDATE_ON_TIME, &http_handler::GameDataBaseHolder::UpdateOnTime_Request} };

};

class Application {

public: 

    Application(std::shared_ptr<http_handler::GameDataBaseHolder> base) : base_(base) {}

    void Tick(std::chrono::milliseconds delta) {
        base_->UpdateOnTick(delta);
    }

private:
    std::shared_ptr<http_handler::GameDataBaseHolder> base_;
};

} // namespace http_handler

namespace boost::json {

http_handler::GameDataBaseHolder::AuthorizeInfo tag_invoke(const value_to_tag<http_handler::GameDataBaseHolder::AuthorizeInfo>&, const value& jv);

} // namespace boost::json