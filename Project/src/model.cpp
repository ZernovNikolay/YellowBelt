#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

void Road::SetBorders() {


    if(end_ < start_) {
        right_up_border.x = static_cast<double>(start_.x) + width;
        right_up_border.y  = static_cast<double>(start_.y) + width;
        left_down_border.x = static_cast<double>(end_.x) - width;
        left_down_border.y = static_cast<double>(end_.y) - width;
    }else{
        right_up_border.x = static_cast<double>(end_.x) + width;
        right_up_border.y  = static_cast<double>(end_.y) + width;
        left_down_border.x = static_cast<double>(start_.x) - width;
        left_down_border.y = static_cast<double>(start_.y) - width;
    }

}

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Map::AddOffices(std::vector<Office> offices) {

    for(auto& v : offices) {
        AddOffice(std::move(v));
    }

}

void Map::AddRoad(const Road& road) {

    roads_.emplace_back(road);

    if(road.IsVertical()) {
        vertical_roads_.insert(road);
    }else{
        horizontal_roads_.insert(road);
    }

}

void Map::AddRoads(const Roads& roads) {

    for(auto& v : roads) { //вот тут копирование, от него надо избавится
        AddRoad(v);
    }

}

std::pair<std::optional<std::set<Road>::iterator>, std::optional<std::set<Road>::iterator>> Map::FindRoads(const Location& location) const{

    std::optional<std::set<Road>::iterator> hor = std::nullopt;
    std::optional<std::set<Road>::iterator> vert = std::nullopt;

    if(auto it = horizontal_roads_.find(location); it != horizontal_roads_.end()) {
        hor = it;
    }

    if(auto it = vertical_roads_.find(location); it != vertical_roads_.end()) {
        vert = it;
    }

    return std::make_pair(hor, vert);

}

void GameSession::UpdateDogLocation(int time, std::shared_ptr<game_unit::Dog> dog) {

    Location location = dog->GetLocation();

    std::pair<std::optional<std::set<Road>::iterator>, std::optional<std::set<Road>::iterator>> old_roads = current_map_->FindRoads(location);

    Location new_location = location + dog->GetSpeed()*(static_cast<double>(time)/1000);

    if(!(old_roads.first.has_value() || old_roads.second.has_value())) {
        return;
    }

    switch (dog->GetDirection())
    {
    case game_unit::DogDirection::DOWN: {
        double max_down = old_roads.second.has_value() ? old_roads.second.value()->GetUpperBorder().y : old_roads.first.value()->GetUpperBorder().y;
        dog->SetLocation({new_location.x, std::abs(new_location.y - location.y) < std::abs(max_down - location.y) ? new_location.y : max_down});
        break;
    }
    
    case game_unit::DogDirection::UP: {
        double max_up = old_roads.second.has_value() ? old_roads.second.value()->GetLowerBorder().y : old_roads.first.value()->GetLowerBorder().y;
        dog->SetLocation({new_location.x, std::abs(new_location.y - location.y) < std::abs(max_up - location.y) ? new_location.y : max_up});
        break;
    }

    case game_unit::DogDirection::LEFT: {
        double max_left = old_roads.first.has_value() ? old_roads.first.value()->GetLowerBorder().x : old_roads.second.value()->GetLowerBorder().x;
        dog->SetLocation({std::abs(new_location.x - location.x) < std::abs(max_left - location.x) ? new_location.x : max_left, new_location.y});
        break;
    }

    case game_unit::DogDirection::RIGHT: {
        double max_right = old_roads.first.has_value() ? old_roads.first.value()->GetUpperBorder().x : old_roads.second.value()->GetUpperBorder().x;
        dog->SetLocation({std::abs(new_location.x - location.x) < std::abs(max_right - location.x) ? new_location.x : max_right, new_location.y});
        break;
    }
    }

    if(dog->GetLocation() != new_location) {
        dog->Stop();
    }

}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            if(map.GetSpeedOnMap() < 0) {
                map.SetSpeedOnMap(standard_speed_);
            }

            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

void Game::AddMaps(std::vector<Map> maps) {

    for(auto& v : maps) {
        AddMap(std::move(v));
    }

}

std::shared_ptr<GameSession> Game::GetGameSession (Map::Id id, bool created) {
    if (auto it = game_sessions_.find(id); it != game_sessions_.end()) {
        return game_sessions_.at(id);
    }
    if(created) {
        if (auto it = FindMap(id)) {
            
            std::shared_ptr<GameSession> out = std::make_shared<GameSession>(it, zoo_);
            AddGameSession(out);
            return out;
            
        }
    }
    return nullptr;
    
}

void Game::UpdateOnTime(int time) {

    for(auto& v: game_sessions_) {
        v.second->UpdateOnTime(time);
    }

}

boost::json::value MakeJsonMap( const model::Map& map ) {

    boost::json::value jv;

    jv = {
            {model::JsonFieldsName::ID, *map.GetId()}, 
            {model::JsonFieldsName::NAME, map.GetName()}, 
            {model::JsonFieldsName::ARRAY_ROADS, boost::json::value_from(map.GetRoads())}, 
            {model::JsonFieldsName::ARRAY_BUILDINGS, boost::json::value_from(map.GetBuildings())}, 
            {model::JsonFieldsName::ARRAY_OFFICES, boost::json::value_from(map.GetOffices())}
        };

    return jv;

}

}  // namespace model

namespace boost::json {

void tag_invoke( const value_from_tag&, value& jv, model::Map const& map) {
    jv = {{model::JsonFieldsName::ID, *map.GetId()}, {model::JsonFieldsName::NAME, map.GetName()}};
}

void tag_invoke( const value_from_tag&, value& jv, model::Office const& office) {
    jv = {
            {model::JsonFieldsName::ID, *office.GetId()},
            {model::JsonFieldsName::X_COORD, office.GetPosition().x},
            {model::JsonFieldsName::Y_COORD, office.GetPosition().y},
            {model::JsonFieldsName::OFFSET_X, office.GetOffset().dx},
            {model::JsonFieldsName::OFFSET_Y, office.GetOffset().dy}
         };
}

void tag_invoke( const value_from_tag&, value& jv, model::Building const& building) {
    jv = value_from(building.GetBounds());
}

void tag_invoke( const value_from_tag&, value& jv, model::Road const& road) {
    if(road.IsHorizontal()) {
        jv = { 
                {model::JsonFieldsName::X_COORD_BEGIN, road.GetStart().x}, 
                {model::JsonFieldsName::Y_COORD_BEGIN, road.GetStart().y}, 
                {model::JsonFieldsName::X_COORD_END, road.GetEnd().x}
             };
    }else if(road.IsVertical()) {
        jv = { 
                {model::JsonFieldsName::X_COORD_BEGIN, road.GetStart().x}, 
                {model::JsonFieldsName::Y_COORD_BEGIN, road.GetStart().y}, 
                {model::JsonFieldsName::Y_COORD_END, road.GetEnd().y}
             };
    }
}

void tag_invoke( const value_from_tag&, value& jv, const model::Road* const& road) {
    if(road->IsHorizontal()) {
        jv = { 
                {model::JsonFieldsName::X_COORD_BEGIN, road->GetStart().x}, 
                {model::JsonFieldsName::Y_COORD_BEGIN, road->GetStart().y}, 
                {model::JsonFieldsName::X_COORD_END, road->GetEnd().x}
             };
    }else if(road->IsVertical()) {
        jv = { 
                {model::JsonFieldsName::X_COORD_BEGIN, road->GetStart().x}, 
                {model::JsonFieldsName::Y_COORD_BEGIN, road->GetStart().y}, 
                {model::JsonFieldsName::Y_COORD_END, road->GetEnd().y}
             };
    }
}

void tag_invoke( const value_from_tag&, value& jv, model::Offset const& offset) {
    jv = {
            {model::JsonFieldsName::OFFSET_X, offset.dx}, 
            {model::JsonFieldsName::OFFSET_Y, offset.dy}
         };
}

void tag_invoke( const value_from_tag&, value& jv, model::Rectangle const& rectangle) {

    jv = {
            {model::JsonFieldsName::X_COORD, rectangle.position.x},
            {model::JsonFieldsName::Y_COORD, rectangle.position.y},
            {model::JsonFieldsName::WEIGHT, rectangle.size.width},
            {model::JsonFieldsName::HEIGHT, rectangle.size.height}
         };
}

model::Game tag_invoke(const value_to_tag<model::Game>&, const value& jv) {

    model::Game new_game;
    if(jv.as_object().if_contains(model::JsonFieldsName::DEFAULT_DOG_SPEED)) {
        new_game.SetSpeedOnGame(boost::json::value_to<double>(jv.at(model::JsonFieldsName::DEFAULT_DOG_SPEED)));
    }else{
        new_game.SetSpeedOnGame(1);
    }
    new_game.AddMaps(boost::json::value_to<std::vector<model::Map>>(jv.at(model::JsonFieldsName::ARRAY_MAPS)));

    return new_game;

}

model::Map tag_invoke(const value_to_tag<model::Map>&, const value& jv) {

    model::Map::Id id_{value_to<std::string>( jv.at( model::JsonFieldsName::ID ) )};
    std::string name_ = value_to<std::string>( jv.at( model::JsonFieldsName::NAME ) );

    model::Map new_map_(id_, name_);

    if(jv.as_object().if_contains(model::JsonFieldsName::DOG_SPEED)) {
        new_map_.SetSpeedOnMap(boost::json::value_to<double>(jv.at(model::JsonFieldsName::DOG_SPEED)));
    }

    new_map_.AddRoads(boost::json::value_to<std::vector<model::Road>>(jv.at( model::JsonFieldsName::ARRAY_ROADS )));
    new_map_.AddBuildings(boost::json::value_to<std::vector<model::Building>>(jv.at( model::JsonFieldsName::ARRAY_BUILDINGS )));
    new_map_.AddOffices(boost::json::value_to<std::vector<model::Office>>(jv.at( model::JsonFieldsName::ARRAY_OFFICES )));

    return new_map_;

}

model::Office tag_invoke(const value_to_tag<model::Office>&, const value& jv) {

    model::Office::Id id_{value_to<std::string>( jv.at( model::JsonFieldsName::ID ) )};
    int x_ = value_to<int>( jv.at( model::JsonFieldsName::X_COORD ) );
    int y_ = value_to<int>( jv.at( model::JsonFieldsName::Y_COORD ) );
    int offsetX_ = value_to<int>( jv.at( model::JsonFieldsName::OFFSET_X ) );
    int offsetY_ = value_to<int>( jv.at( model::JsonFieldsName::OFFSET_Y ) );
    return model::Office{id_, {x_, y_}, {offsetX_, offsetY_}};

}

model::Building tag_invoke(const value_to_tag<model::Building>&, const value& jv) {

    return model::Building(value_to<model::Rectangle>(jv));

}

model::Road tag_invoke(const value_to_tag<model::Road>&, const value& jv) {

    int x_ = value_to<int>( jv.at( model::JsonFieldsName::X_COORD_BEGIN ) );
    int y_ = value_to<int>( jv.at( model::JsonFieldsName::Y_COORD_BEGIN ) );

    if(jv.as_object().if_contains(model::JsonFieldsName::Y_COORD_END)) {
        return model::Road{ model::Road::VERTICAL, {x_, y_}, value_to<int>( jv.at( model::JsonFieldsName::Y_COORD_END ) )};
    }

    if(jv.as_object().if_contains(model::JsonFieldsName::X_COORD_END)) {
        return { model::Road::HORIZONTAL, {x_, y_}, value_to<int>( jv.at( model::JsonFieldsName::X_COORD_END ) )};
    };

    return model::Road{model::Road::HORIZONTAL, {0, 0}, 0};

}

model::Rectangle tag_invoke(const value_to_tag<model::Rectangle>&, const value& jv) {

    int x_ = value_to<int>( jv.at( model::JsonFieldsName::X_COORD ) );
    int y_ = value_to<int>( jv.at( model::JsonFieldsName::Y_COORD ) );
    int width_ = value_to<int>( jv.at( model::JsonFieldsName::WEIGHT ) );
    int height_ = value_to<int>( jv.at( model::JsonFieldsName::HEIGHT ) );

    return model::Rectangle({x_, y_}, {width_, height_});

}

}   // namespase boost::json
