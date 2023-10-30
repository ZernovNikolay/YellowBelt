#include "info_holders.h"

namespace http_handler {

void CheckMethod(const std::string_view allowed, const std::string_view target) {

    if(allowed.find(target) == std::string_view::npos) {
        throw RequestException(ErrorType::INVALID_METHOD);
    }

}

char CheckDirection(const std::string& body) {

    boost::json::value jv = boost::json::parse(body);

    if(jv.as_object().if_contains(request_fields::MOVE)) {
        
        if(std::string dir = boost::json::value_to<std::string>(jv.at(request_fields::MOVE));  possible_directions.find(dir) != possible_directions.end() ) {

            if(dir.size() > 0) {
                return dir[0];
            }
            return 'S';
        }

    }

    throw RequestException(ErrorType::INVALID_MOVE);
}

int CheckTimeTick(const std::string& body) {

    if(!body.empty()) {

        boost::json::value jv = boost::json::parse(body);

        if(jv.as_object().if_contains(request_fields::TIME_TICK)) {

            if(jv.as_object().at(request_fields::TIME_TICK).is_int64()) {

                return boost::json::value_to<int>(jv.at(request_fields::TIME_TICK));

            }
        }
    }

    throw RequestException(ErrorType::INVALID_TICK_TIME);
    
}

http_handler::GameDataBaseHolder::AuthorizeInfo CheckInitialAuthorize(const std::string& authorize_field) {

    boost::json::value jv = boost::json::parse(authorize_field);

    if(jv.as_object().if_contains(request_fields::MAP_ID) && jv.as_object().if_contains(request_fields::USER_NAME)) {

        http_handler::GameDataBaseHolder::AuthorizeInfo info = boost::json::value_to<http_handler::GameDataBaseHolder::AuthorizeInfo>(jv);

        if(info.new_name == request_fields::INCORRECT_USER_NAME) {
             //невозможное имя
            throw RequestException(ErrorType::INVALID_ARGUMENT_NAME);        
        }

        return info;

    }else{
        throw RequestException(ErrorType::INVALID_ARGUMENT_PARSING);
    }
}

FileResponse MakeFileResponse(http::status status, http::file_body::value_type file_body, 
                              RequestInfo& info_req, std::string_view content_type) {

    FileResponse response(status, info_req.http_version);
    response.set(http::field::content_type, content_type);
    response.keep_alive(info_req.keep_alive);

    if(info_req.method  != RequestMethod::HEAD) {
        response.body() = std::move(file_body);
        response.prepare_payload();
    }

    return response;
}

StringResponse MakeStringResponse(http::status status, std::string_view body,
                                  RequestInfo& info_req, std::string_view content_type) {

    if(content_type.size() < 3){
        std::cout << "ERROR" << std::endl;
    }

    StringResponse response(status, info_req.http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(info_req.keep_alive);
    response.set(http::field::cache_control, "no-cache");
    if(status == http::status::method_not_allowed) {
        response.set(http::field::allow, info_req.method_allowed);
    }

    return response;
}

StringResponse ErrorHandle(ErrorType error, RequestInfo& info_req) {
    
    switch (error)
    {
    case ErrorType::INVALID_TOKEN:
        return MakeStringResponse(http::status::unauthorized, string_response::INVALID_TOKEN, info_req, ContentType::JSON_DATA);
        break;
    
    case ErrorType::UNKNOWN_TOKEN:
        return MakeStringResponse(http::status::unauthorized, string_response::UNKNOWN_TOKEN, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::INVALID_METHOD:
        return MakeStringResponse(http::status::method_not_allowed, string_response::INVALID_METHOD, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::MAP_NOT_FOUND:
        return MakeStringResponse(http::status::not_found, string_response::MAP_NOT_FOUND, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::BAD_REQUEST:
        return MakeStringResponse(http::status::bad_request, string_response::BAD_REQUEST, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::INVALID_ARGUMENT_NAME:
        return MakeStringResponse(http::status::bad_request, string_response::INVALID_ARGUMENT_NAME, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::INVALID_ARGUMENT_PARSING:
        return MakeStringResponse(http::status::bad_request, string_response::INVALID_ARGUMENT_PARSING, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::FILE_NOT_FOUND:
        return MakeStringResponse(http::status::not_found, string_response::FILE_NOT_FOUND, info_req, ContentType::TEXT_PLAIN);
        break;

    case ErrorType::INVALID_MOVE:
        return MakeStringResponse(http::status::bad_request, string_response::INVALID_MOVE, info_req, ContentType::JSON_DATA);
        break;

    case ErrorType::INVALID_TICK_TIME:
        return MakeStringResponse(http::status::bad_request, string_response::INVALID_TICK_TIME, info_req, ContentType::JSON_DATA);
        break;   

    default:
        return MakeStringResponse(http::status::failed_dependency, string_response::SERVER_ERROR, info_req, ContentType::JSON_DATA);
        break;
    }

}

StringResponse GameDataBaseHolder::HandleURI(RequestInfo& info_req) {

    try{

        if(info_req.target.starts_with(TargetType::ADRESS_MAP)) {

            info_req.target.remove_prefix(TargetType::ADRESS_MAP.size());

            if(info_req.target.empty()) {

                return Game_Request(Request_API::FULL_INFO_REQUEST, info_req);

            }else{
                info_req.target.remove_prefix(1);
                return Game_Request(Request_API::CERTAIN_MAP_REQUEST, info_req);
            }
        }

        auto it = methods_library.find(info_req.target);

        if(it == methods_library.end()) {
            throw RequestException(ErrorType::BAD_REQUEST);
        }

        return (this->*(it->second))(info_req);

    } catch(const RequestException& error){
        return ErrorHandle(error.getMessage(), info_req);
    }

}

void GameDataBaseHolder::UpdateOnTick(std::chrono::milliseconds delta) {

    game_.UpdateOnTime(delta.count());

}

std::shared_ptr<game_unit::Player> GameDataBaseHolder::Authorization_Process(const std::string_view& authorize_field) {

    std::string token;
    if(authorize_field.starts_with(request_fields::BEARER_)) {
        token.append(authorize_field.begin() + 7, authorize_field.end());
        if(token.size() != 32) {
            throw RequestException(ErrorType::INVALID_TOKEN);        
        }
    }else{
        throw RequestException(ErrorType::INVALID_TOKEN);        
    }

    if(auto it = players_->FindPlayer(token)) {

        return it;

    }

    throw RequestException(ErrorType::UNKNOWN_TOKEN);
}

StringResponse GameDataBaseHolder::Game_Request(Request_API request, RequestInfo& info_req) {

    info_req.method_allowed = RequestMethod::ALLOW_GET_HEAD;
    CheckMethod(info_req.method_allowed, info_req.method);

    switch (request) {

        case(Request_API::CERTAIN_MAP_REQUEST):
        {

            const model::Map::Id id_map_({info_req.target.begin(), info_req.target.end()});
            if(auto t = game_.FindMap(id_map_); t != nullptr) {

                std::string out = boost::json::serialize(model::MakeJsonMap(*t));
                return MakeStringResponse(http::status::ok, out, info_req, ContentType::JSON_DATA);

            }else{ 
                throw RequestException(ErrorType::MAP_NOT_FOUND);
            }
            break;
        }

        case(Request_API::FULL_INFO_REQUEST):
        {
            std::vector<model::Map> maps = game_.GetMaps();
            return MakeStringResponse(http::status::ok, boost::json::serialize(boost::json::value_from(maps)), info_req, ContentType::JSON_DATA);
        }

    }

    return MakeStringResponse(http::status::failed_dependency, string_response::SERVER_ERROR, info_req, ContentType::JSON_DATA);
}

StringResponse GameDataBaseHolder::Authorize_Request(RequestInfo& info_req) {

    info_req.method_allowed = RequestMethod::ALLOW_POST;
    CheckMethod(info_req.method_allowed, info_req.method);
    http_handler::GameDataBaseHolder::AuthorizeInfo info = CheckInitialAuthorize(info_req.body);

    std::shared_ptr<game_unit::Player> new_player = CreateNewPlayer(info);

    boost::json::value jv = {
                                {response_fields::AUTH_TOKEN, new_player->GetToken()}, 
                                {response_fields::PLAYER_ID, new_player->GetId()}, 
                            };

    return MakeStringResponse(http::status::ok, boost::json::serialize(jv), info_req, ContentType::JSON_DATA);

}

StringResponse GameDataBaseHolder::PlayerList_Request(RequestInfo& info_req) {

    info_req.method_allowed = RequestMethod::ALLOW_GET_HEAD;

    CheckMethod(info_req.method_allowed, info_req.method);

    std::shared_ptr<game_unit::Player> desired = Authorization_Process(info_req.authorize);

    std::unordered_set<std::shared_ptr<game_unit::Player>> out = players_->GetPlayerList(desired->GetSession());

    boost::json::object tg;

    for(auto& v : out){
        tg.emplace(std::to_string(v->GetId()), boost::json::value_from(v));
    }

    return MakeStringResponse(http::status::ok, boost::json::serialize(tg), info_req, ContentType::JSON_DATA);
}

StringResponse GameDataBaseHolder::GameState_Request(RequestInfo& info_req) {


    info_req.method_allowed = RequestMethod::ALLOW_GET_HEAD;
    CheckMethod(info_req.method_allowed, info_req.method);
    std::shared_ptr<game_unit::Player> desired = Authorization_Process(info_req.authorize);

    std::shared_ptr<model::GameSession> session = desired->GetSession();
    std::unordered_map<std::string, std::shared_ptr<game_unit::Dog>>& tt = session->GetDogList();

    boost::json::object jv;
    jv.emplace(response_fields::PLAYERS, boost::json::value_from(tt));

    return MakeStringResponse(http::status::ok, boost::json::serialize(jv), info_req, ContentType::JSON_DATA);
}

StringResponse GameDataBaseHolder::Move_Request(RequestInfo& info_req) {

    info_req.method_allowed = RequestMethod::ALLOW_POST;
    CheckMethod(info_req.method_allowed, info_req.method);
    std::shared_ptr<game_unit::Player> desired = Authorization_Process(info_req.authorize);

    std::shared_ptr<game_unit::Dog> dog = desired->GetDog();
    dog->SetDirection(CheckDirection(info_req.body));

    return MakeStringResponse(http::status::ok, string_response::EMPTY_ANSWER, info_req, ContentType::JSON_DATA);

}

StringResponse GameDataBaseHolder::UpdateOnTime_Request(RequestInfo& info_req) {

    if(!active_time_management) {
        throw RequestException(ErrorType::BAD_REQUEST);
    }

    info_req.method_allowed = RequestMethod::ALLOW_POST;
    CheckMethod(info_req.method_allowed, info_req.method);

    game_.UpdateOnTime(CheckTimeTick(info_req.body));

    return MakeStringResponse(http::status::ok, string_response::EMPTY_ANSWER, info_req, ContentType::JSON_DATA);
}

std::shared_ptr<game_unit::Player> GameDataBaseHolder::CreateNewPlayer(AuthorizeInfo info) {

    std::shared_ptr<model::GameSession> current_session = game_.GetGameSession(info.map_id, true);

    if(current_session == nullptr) {
        throw RequestException(ErrorType::MAP_NOT_FOUND);
    }

    std::shared_ptr<game_unit::Dog> dog = current_session->CreateDog(info.new_name);

    if(randomize_start_point) {

        //сгенерируем рандомную точку. Это просто начало случайной дороги, а нам большего и не нужно

        int roads = game_.FindMap(info.map_id)->GetRoads().size();
        int t = std::experimental::randint(0, roads-1);

        model::Point start_point = game_.FindMap(info.map_id)->GetRoads()[t].GetStart();

        dog->SetLocation( {static_cast<double>(start_point.x), static_cast<double>(start_point.y) } );

    }

    return players_->AddNewPlayer(dog, current_session, tokenizer_->GetToken());

}

} //namespace http_handler

namespace boost::json {

http_handler::GameDataBaseHolder::AuthorizeInfo tag_invoke(const value_to_tag<http_handler::GameDataBaseHolder::AuthorizeInfo>&, const value& jv) {

    model::Map::Id id_{value_to<std::string>( jv.at( http_handler::request_fields::MAP_ID ) )};
    std::string user_name_ = value_to<std::string>( jv.at( http_handler::request_fields::USER_NAME ) );
    return http_handler::GameDataBaseHolder::AuthorizeInfo{id_, user_name_};

}

} // namespace boost::json