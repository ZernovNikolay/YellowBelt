#include "player.h"

namespace game_unit {

std::string Tokenizer::GetToken() {

    int64_t a = generator1_();
    int64_t b = generator2_();

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(16) << std::hex << a;

    stream << std::setfill('0') << std::setw(16) << std::hex << b;

    std::string result( stream.str() );

    return result;
}

std::shared_ptr<game_unit::Dog> Player::GetDog() {
    return dog_;
}

std::shared_ptr<Player> PlayerLibrary::AddNewPlayer(std::shared_ptr<game_unit::Dog> dog, std::shared_ptr<model::GameSession> game_session, Token new_token) {

    //сделать проверку на то, что такого токена ещё нет

    std::shared_ptr new_player = std::make_shared<Player>(dog, game_session, new_token);

    players.insert({new_token, new_player});

    game_sessions[game_session].insert(new_player);

    return new_player;

}

std::shared_ptr<Player> PlayerLibrary::FindPlayer(Token token) {

    if(auto it = players.find(token); it != players.end()) {

        return it->second;

    }else{
        return nullptr;
    }

}

} // namespace game_unit

namespace boost::json {

void tag_invoke( const value_from_tag&, value& jv, std::shared_ptr<game_unit::Player> const& player) {

    jv = {
            {"name", player->GetName()}
        };

}

} // namespace boost::json