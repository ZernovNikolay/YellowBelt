#pragma once

#include "model.h"
#include <random>
#include <unordered_set>

namespace game_unit {

class Tokenizer{

public:

    std::string GetToken();

private:

    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

};

//id игрока - это id его собаки
class Player {
    using Token = std::string;
public:

    Player(std::shared_ptr<game_unit::Dog> dog, std::shared_ptr<model::GameSession> game_session, Token new_token) noexcept
        : dog_(dog)
        , game_session_(game_session)
        , token_(new_token) {

            dog_->SetStandardSpeed(game_session_->GetSpeedOnMap());

        }

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;


    std::shared_ptr<game_unit::Dog> GetDog();

    int GetId() const{

        return dog_->GetId();

    }

    std::string GetName() const {
        return dog_->GetName();
    }

    std::shared_ptr<model::GameSession> GetSession() {
        return game_session_;
    }

    const Token& GetToken() const {
        return token_;
    }

private:

    std::shared_ptr<game_unit::Dog> dog_;
    std::shared_ptr<model::GameSession> game_session_;
    Token token_;

};

class PlayerLibrary {

    using Token = std::string;

public:

    [[nodiscard]] std::shared_ptr<Player> AddNewPlayer(std::shared_ptr<game_unit::Dog> dog, std::shared_ptr<model::GameSession> game_session, Token new_token);

    std::shared_ptr<Player> FindPlayer(Token token);

    std::unordered_set<std::shared_ptr<Player>>& GetPlayerList(const std::shared_ptr<model::GameSession>& finder) {

        return game_sessions.at(finder);

    }

private:

    std::unordered_map<Token, std::shared_ptr<Player>> players;

    std::unordered_map<std::shared_ptr<model::GameSession>, std::unordered_set<std::shared_ptr<Player>>> game_sessions; 
    
};

} // namespace game_unit

namespace boost::json {

void tag_invoke( const value_from_tag&, value& jv, std::shared_ptr<game_unit::Player> const& player);

} // namespace boost::json