#include "json_loader.h"

namespace json_loader {

namespace json = boost::json;

std::string read_file(const std::filesystem::path& json_path) {

    std::ifstream stream(json_path);
    const auto size_file = std::filesystem::file_size(json_path);
    std::string result(size_file, '\0');
    stream.read(result.data(), size_file);

    return result;
}

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла

    boost::json::value jv = boost::json::parse(read_file(json_path));
    model::Game game = boost::json::value_to<model::Game>(jv);
    
    return game;
}

}  // namespace json_loader
