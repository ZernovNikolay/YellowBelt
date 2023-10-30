#pragma once

#include <unordered_map>
#include <set>
#include <vector>
#include <utility>
#include <optional>
#include <cmath>

#include "tagged.h"
#include "dog.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct JsonFieldsName {

    JsonFieldsName() = delete;
    inline static const std::string X_COORD = "x";
    inline static const std::string Y_COORD = "y";
    inline static const std::string X_COORD_BEGIN = "x0";
    inline static const std::string X_COORD_END = "x1";
    inline static const std::string Y_COORD_BEGIN = "y0";
    inline static const std::string Y_COORD_END = "y1";
    inline static const std::string WEIGHT = "w";
    inline static const std::string HEIGHT = "h";
    inline static const std::string OFFSET_X = "offsetX";
    inline static const std::string OFFSET_Y = "offsetY";
    inline static const std::string ID = "id";
    inline static const std::string NAME = "name";
    inline static const std::string ARRAY_ROADS = "roads";
    inline static const std::string ARRAY_BUILDINGS = "buildings";
    inline static const std::string ARRAY_OFFICES = "offices";
    inline static const std::string ARRAY_MAPS = "maps";
    inline static const std::string DEFAULT_DOG_SPEED = "defaultDogSpeed";
    inline static const std::string DOG_SPEED = "dogSpeed";

};

struct Point {
    Coord x, y;

    bool operator<(const Point& a) const {
        return x < a.x || y < a.y;
    }
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:

    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
            SetBorders();
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
            SetBorders();
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    Location GetLowerBorder() const noexcept {
        return left_down_border;
    }

    Location GetUpperBorder() const noexcept {
        return right_up_border;
    }

    bool operator==(const Road& a) const {
        return (right_up_border == a.GetUpperBorder()) && (left_down_border == a.GetLowerBorder());
    }

private:

    void SetBorders();

    double width = 0.4;

    Point start_;
    Point end_;

    Location right_up_border = {0, 0};
    Location left_down_border = {0, 0};

    // стоит ли менять конструктор чтобы было сразу понятно где начало, где конец (выше/ниже)? 
    // Не думаю, ведь мы явно обозначим углы границ и этого хватит для дальнейшей обработки
    // что так, что так всего одна проверка
};

class Building {
public:

    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road);

    void AddRoads(const Roads& roads);

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddBuildings(const Buildings& buildings) {
        buildings_.insert(buildings_.end(), buildings.begin(), buildings.end());
    }

    void AddOffice(Office office);

    void AddOffices(std::vector<Office> offices);

    void SetSpeedOnMap(double speed)  {
        standard_speed_ = speed;
    }

    double GetSpeedOnMap() const{
        return standard_speed_;
    }

    std::pair<std::optional<std::set<Road>::iterator>, std::optional<std::set<Road>::iterator>> FindRoads(const Location& location) const;
private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    struct Comparator_horizontal
    {
        using is_transparent = void;

        bool operator()(const Road& lhs, const Road& rhs) const {

            if(lhs.GetUpperBorder().y != rhs.GetUpperBorder().y) {
                return lhs.GetUpperBorder().y < rhs.GetUpperBorder().y;
            }else{
                return lhs.GetUpperBorder().x < rhs.GetUpperBorder().x;
            }          
        }

        bool operator()(const Location& point, const Road& road) const {

            if(point.y <= road.GetUpperBorder().y) {
                return point.y < road.GetLowerBorder().y || point.x < road.GetLowerBorder().x;
            }else{
                return false;
            } 
        }

        bool operator()(const Road& road, const Location& point) const {

            if(road.GetLowerBorder().y <= point.y) {
                return road.GetUpperBorder().y < point.y || road.GetUpperBorder().x < point.x;
            }else{
                return false;
            }
        }
    };

    struct Comparator_vertical
    {
        using is_transparent = void;

        bool operator()(const Road& lhs, const Road& rhs) const{
            if(lhs.GetUpperBorder().x != rhs.GetUpperBorder().x) {
                return lhs.GetUpperBorder().x < rhs.GetUpperBorder().x;
            }else{
                return lhs.GetUpperBorder().y < rhs.GetUpperBorder().y;
            }
        }

        bool operator()(const Location& point, const Road& road) const {

            if(point.x <= road.GetUpperBorder().x) {
                return point.y < road.GetLowerBorder().y || point.x < road.GetLowerBorder().x;
            }else{
                return false;
            }
        }

        bool operator()(const Road& road, const Location& point) const {

            if(road.GetLowerBorder().x <= point.x) {
                return road.GetUpperBorder().x < point.x || road.GetUpperBorder().y < point.y;
            }else{
                return false;
            }
        }
    };


    //для того чтобы быстрее искать дорогу по точке, можно разделить все дороги на 2 типа - горизонтальные и вертикальные
    //Тогда у нас будет два поиска по двум различным set<Road, Comparator> (один компаратор по x для вертикальных, один по y для горизонтальных)
    //Это значительно ускорит поиск
    //Для такой же логики заранее добавить к каждой дороге её границы (внутри класса) (ведь я буду ими постоянно пользоваться)
    //Но я оставляю дороги в векторе, чтобы лишний раз не сломать вывод boost и tag_invoke, поэтому в set у меня будут ссылки на Road

    //так делается, потому что дороги являются неотъемлемой частью карты и значит сортировку необходимо и лучше всего делать внутри карты 

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;

    double standard_speed_ = -100; //отрицательной быть не может, поэтому при добавлении в игру игра проверит это и подставит свою скорость

    std::set<Road, Comparator_horizontal> horizontal_roads_;
    std::set<Road, Comparator_vertical> vertical_roads_; 
};

class GameSession {

public:
    GameSession(const Map* map, std::shared_ptr<game_unit::Zoo> zoo) 
        : current_map_(map)
        , zoo_(zoo) {}

    [[nodiscard]] std::shared_ptr<game_unit::Dog> CreateDog(const std::string& name) {

        std::shared_ptr<game_unit::Dog> new_dog = zoo_->GetDog(name);
        Add_Dog(new_dog);
        return new_dog;

    }

    std::unordered_map<std::string, std::shared_ptr<game_unit::Dog>>& GetDogList() {
        return dogs_;
    }

    Map::Id GetIdMap() {
        return current_map_->GetId();
    }

    double GetSpeedOnMap() const{
        return current_map_->GetSpeedOnMap();
    }

    void UpdateOnTime(int time) {

        for(auto& v : dogs_) {
            UpdateDogLocation(time, v.second);
        }

    }

private:

    void Add_Dog(std::shared_ptr<game_unit::Dog> new_dog) {
        if(dogs_.find(std::to_string(new_dog->GetId())) == dogs_.end()) {
            dogs_.insert({std::to_string(new_dog->GetId()), new_dog});
        }
    }

    void UpdateDogLocation(int time, std::shared_ptr<game_unit::Dog> dog);

    const Map* current_map_;
    std::unordered_map<std::string, std::shared_ptr<game_unit::Dog>> dogs_;
    std::shared_ptr<game_unit::Zoo> zoo_;

};

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    // не const, потому что AddMap принимает не const Map
    void AddMaps(std::vector<Map> maps);

    void AddGameSession (std::shared_ptr<GameSession> game_session) {
        game_sessions_.insert({game_session->GetIdMap(), game_session});
    }


    // поставить bool для того чтобы если пусто, то создать
    std::shared_ptr<GameSession> GetGameSession (Map::Id id, bool created);

    void SetSpeedOnGame(double speed) {
        standard_speed_ = speed;
    }

    double GetSpeedOnGame() const {
        return standard_speed_;
    }

    void UpdateOnTime(int time);

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    using GameSessions = std::unordered_map<Map::Id, std::shared_ptr<GameSession>, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    GameSessions game_sessions_;
    std::shared_ptr<game_unit::Zoo> zoo_ = std::make_shared<game_unit::Zoo>();

    double standard_speed_;
};

boost::json::value MakeJsonMap( const model::Map& map );

}  // namespace model

namespace boost::json {

void tag_invoke( const value_from_tag&, value& jv, model::Map const& map);

void tag_invoke( const value_from_tag&, value& jv, model::Office const& office);

void tag_invoke( const value_from_tag&, value& jv, model::Building const& building);

void tag_invoke( const value_from_tag&, value& jv, model::Road const& road);

void tag_invoke( const value_from_tag&, value& jv, const model::Road* const& road);

void tag_invoke( const value_from_tag&, value& jv, model::Offset const& offset);

void tag_invoke( const value_from_tag&, value& jv, model::Rectangle const& rectangle);

model::Game tag_invoke(const value_to_tag<model::Game>&, const value& jv);

model::Map tag_invoke(const value_to_tag<model::Map>&, const value& jv);

model::Office tag_invoke(const value_to_tag<model::Office>&, const value& jv);

model::Building tag_invoke(const value_to_tag<model::Building>&, const value& jv);

model::Road tag_invoke(const value_to_tag<model::Road>&, const value& jv);

model::Rectangle tag_invoke(const value_to_tag<model::Rectangle>&, const value& jv);

} // namespase boost::json