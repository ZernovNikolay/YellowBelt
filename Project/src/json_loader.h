#pragma once

#include <filesystem>
#include <fstream>

#include <iostream>

#include "model.h"

namespace json_loader {

// одинокая строка ошибки при открытии файла
const std::string FILE_NOT_EXIST = "file not exist";

// функция загрузки файла
std::string read_file(const std::filesystem::path& json_path);

model::Game LoadGame(const std::filesystem::path& json_path);

}  // namespace json_loader
