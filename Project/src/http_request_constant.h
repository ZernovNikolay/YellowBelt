#pragma once

#include <boost/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace http_handler {

using namespace std::literals;

namespace json_response {

// Стандартные тела ответов на все случаи жизни

const boost::json::value BAD_REQUEST = {
                                            {"code", "badRequest"},
                                            {"message", "Bad request"}
                                        };

const boost::json::value MAP_NOT_FOUND = {
                                            {"code", "mapNotFound"},
                                            {"message", "Map not found"}
                                        };

const boost::json::value INVALID_METHOD = {
                                            {"code", "invalidMethod"},
                                            {"message", "Invalid method"}
                                        };

const boost::json::value SERVER_ERROR = {
                                            {"code", "ServerError"},
                                            {"message", "Server Error"}
                                        };

const boost::json::value INVALID_ARGUMENT_NAME = {
                                                    {"code", "invalidArgument"},
                                                    {"message", "Invalid name"}
                                                };

const boost::json::value INVALID_ARGUMENT_PARSING = {
                                                        {"code", "invalidArgument"},
                                                        {"message", "Join game request parse error"}
                                                    };

const boost::json::value INVALID_TOKEN = {
                                            {"code", "invalidToken"},
                                            {"message", "Authorization header is missing"}
                                        };

const boost::json::value UNKNOWN_TOKEN = {
                                            {"code", "unknownToken"},
                                            {"message", "Player token has not been found"}
                                        };

const boost::json::value INVALID_MOVE = {
                                            {"code", "invalidArgument"},
                                            {"message", "Failed to parse action"}
                                        };

const boost::json::value INVALID_TICK_TIME = {
                                            {"code", "invalidArgument"},
                                            {"message", "Failed to parse tick request JSON"}
                                        };

} // namespace json_response

namespace string_response {

const std::string EMPTY_ANSWER = "{}";

const std::string FILE_NOT_FOUND = "NOTHING HERE WITH THIS NAME";

const std::string INVALID_TICK_TIME = boost::json::serialize(json_response::INVALID_TICK_TIME);

const std::string BAD_REQUEST = boost::json::serialize(json_response::BAD_REQUEST);

const std::string MAP_NOT_FOUND = boost::json::serialize(json_response::MAP_NOT_FOUND);

const std::string INVALID_METHOD = boost::json::serialize(json_response::INVALID_METHOD);

const std::string SERVER_ERROR = boost::json::serialize(json_response::SERVER_ERROR);

const std::string INVALID_ARGUMENT_NAME = boost::json::serialize(json_response::INVALID_ARGUMENT_NAME);

const std::string INVALID_ARGUMENT_PARSING = boost::json::serialize(json_response::INVALID_ARGUMENT_PARSING);

const std::string INVALID_TOKEN = boost::json::serialize(json_response::INVALID_TOKEN);

const std::string UNKNOWN_TOKEN = boost::json::serialize(json_response::UNKNOWN_TOKEN);

const std::string INVALID_MOVE = boost::json::serialize(json_response::INVALID_MOVE);

}

// То, что хотим видеть в методе
struct RequestMethod {
    RequestMethod() = delete;

    constexpr static std::string_view HEAD = "HEAD"sv;
    constexpr static std::string_view GET = "GET"sv;
    constexpr static std::string_view POST = "POST"sv;
    constexpr static std::string_view ALLOW_GET_HEAD = "GET, HEAD"sv;
    constexpr static std::string_view ALLOW_POST = "POST"sv;
};


// То, что на самом деле передаём в теле ответа
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON_DATA = "application/json"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JS = "text/javascript"sv;
    constexpr static std::string_view XML_DATA = "application/xml"sv;
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;
    constexpr static std::string_view AUDIO_MP3 = "audio/mpeg"sv;
    constexpr static std::string_view UNKNOWN_TYPE = "application/octet-stream"sv;

    inline const static std::unordered_map<std::string_view, std::string_view> Extension_type = { 
        {".htm"sv, TEXT_HTML},
        {".html"sv, TEXT_HTML},
        {".css"sv, TEXT_CSS},
        {".txt"sv, TEXT_PLAIN},
        {".js"sv, TEXT_JS},
        {".json"sv, JSON_DATA},
        {".xml"sv, XML_DATA},
        {".png"sv, IMAGE_PNG},
        {".jpg", IMAGE_JPEG},
        {".jpeg", IMAGE_JPEG},
        {".jpe", IMAGE_JPEG},
        {".gif", IMAGE_GIF},
        {",bmp", IMAGE_BMP},
        {".ico", IMAGE_ICO},
        {".tiff", IMAGE_TIF},
        {".tif", IMAGE_TIF},
        {".svg", IMAGE_SVG},
        {".svgz", IMAGE_SVG},
        {".mp3", AUDIO_MP3}
    };

    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

// То, что хотим видеть в target'е запроса
struct TargetType {
    TargetType() = delete;
    constexpr static std::string_view ADRESS_API = "/api"sv;           // адрес игры
    constexpr static std::string_view ADRESS_MAP = "/api/v1/maps"sv;    // адрес карт внутри игры
    constexpr static std::string_view ADRESS_JOIN_POINT = "/api/v1/game/join"sv;
    constexpr static std::string_view ADRESS_PLAYER_LIST = "/api/v1/game/players"sv;
    constexpr static std::string_view ADRESS_GAME_STATE = "/api/v1/game/state"sv;
    constexpr static std::string_view ADRESS_MOVE_PLAYER = "/api/v1/game/player/action"sv;
    constexpr static std::string_view UPDATE_ON_TIME = "/api/v1/game/tick"sv;
};

enum Request_API {
    CERTAIN_MAP_REQUEST,
    FULL_INFO_REQUEST
};

enum ErrorType {
    UNKNOWN_TOKEN,
    INVALID_TOKEN,
    INVALID_METHOD,
    INVALID_ARGUMENT_NAME,
    INVALID_ARGUMENT_PARSING,
    MAP_NOT_FOUND,
    BAD_REQUEST,
    FILE_NOT_FOUND,
    SERVER_ERROR,
    INVALID_MOVE,
    INVALID_TICK_TIME
};

static const std::unordered_set<std::string> possible_directions = {"U", "D", "L", "R", ""};

namespace request_fields {

const std::string MOVE = "move";
const std::string TIME_TICK = "timeDelta";
const std::string MAP_ID = "mapId";
const std::string USER_NAME = "userName";

const std::string INCORRECT_USER_NAME = "";

constexpr std::string_view BEARER_ = "Bearer "sv;

}

namespace response_fields {

const std::string PLAYERS = "players";
const std::string AUTH_TOKEN = "authToken";
const std::string PLAYER_ID = "playerId";

}

} // namespace http_handler