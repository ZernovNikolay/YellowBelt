#pragma once

#include <string>
#include <boost/json.hpp>

namespace model {

struct Location { //вдруг оказалось что дорога целочисленная, а её ширина нет(
    double x, y;

    Location operator+(const Location& b) const {
        return {x + b.x, y + b.y};
    }

    bool operator==(const Location& b) const {
        return x == b.x && y == b.y;
    }

};

} // namespace model

namespace game_unit {

struct JsonFieldsName {

    JsonFieldsName() = delete;
    inline static const std::string SPEED = "speed";
    inline static const std::string POSITION = "pos";
    inline static const std::string DIRECTION = "dir";

};

struct DogDirection{

    DogDirection() = delete;
    inline static const char UP = 'U';
    inline static const char DOWN = 'D';
    inline static const char LEFT = 'L';
    inline static const char RIGHT = 'R';
    inline static const char STOP = 'S';

};

class Dog {

public:

    struct Speed {
        
        double speed_x;
        double speed_y;

        model::Location operator*(double time) {

            return {speed_x*time, speed_y*time};

        }

    };

    Dog(const std::string& name, int id): id_(id), name_(name) {}

    Dog(const Dog&) = delete;
    Dog& operator=(const Dog&) = delete;

    int GetId() const {
        return id_;
    }

    std::string GetName() const {
        return name_;
    }

    Speed GetSpeed() const {
        return speed_;
    }

    model::Location GetLocation() const {
        return location_;
    }

    void SetLocation(const model::Location& location) {
        location_ = location;
    }

    char GetDirection() const {
        return direction_;
    }

    std::string GetStringDirection() const {

        if(direction_ == DogDirection::STOP){
            return "";
        }

        return std::string{direction_};
    }

    void SetDirection(char direction);

    void SetStandardSpeed(double speed) {
        standard_speed_ = speed;
    }

    void Stop() {
        speed_.speed_x = 0;
        speed_.speed_y = 0;
    }

private:

    void SetSpeed(double x, double y) {
        speed_.speed_x = x;
        speed_.speed_y = y;
    }

    const int id_;
    const std::string name_;
    Speed speed_{0, 0};
    model::Location location_{0, 0};
    char direction_ = DogDirection::UP;
    double standard_speed_ = -100;

};

class Zoo {

public:
    std::shared_ptr<Dog> GetDog(const std::string& name) {
        {
            std::lock_guard<std::mutex> lock(store_mutex);
            return std::make_shared<Dog>(name, ++next_id_);
        }

    }

private:
    int next_id_ = 0;
    std::mutex store_mutex;

};

} // namespace game_unit

namespace boost::json {
    void tag_invoke( const value_from_tag&, value& jv, std::shared_ptr<game_unit::Dog> const& player);
} // namespace boost::json