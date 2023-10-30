#include "dog.h"

namespace game_unit {

    void Dog::SetDirection(char direction) {

        direction_ = direction;

        switch (direction_)
        {
        case DogDirection::UP:
            SetSpeed(0, -standard_speed_);
            break;
        
        case DogDirection::DOWN:
            SetSpeed(0, standard_speed_);
            break;

        case DogDirection::LEFT:
            SetSpeed(-standard_speed_, 0);
            break;

        case DogDirection::RIGHT:
            SetSpeed(standard_speed_, 0);
            break;

        case DogDirection::STOP:
            Stop();
            break;
        }

    }
} // namespace game_unit

namespace boost::json {

void tag_invoke( const value_from_tag&, value& jv, std::shared_ptr<game_unit::Dog> const& player) {

    jv = {                              
            {game_unit::JsonFieldsName::POSITION, {player->GetLocation().x, player->GetLocation().y }},
            {game_unit::JsonFieldsName::SPEED, {player->GetSpeed().speed_x, player->GetSpeed().speed_y}},
            {game_unit::JsonFieldsName::DIRECTION, std::string{player->GetStringDirection()}}                 
    };

}

} // namespace boost::json