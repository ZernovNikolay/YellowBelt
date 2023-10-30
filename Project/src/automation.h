#pragma once

#include <boost/program_options.hpp>
#include <exception>
#include <string>
#include <optional>
#include <iostream>

struct Args {
    int period = 0;                       // период таймера
    bool random = false;                  // рандомизируем стартовую точку или нет
    bool passive_time_managment = false;  // включен ли таймер
    std::string config;                   // путь до файла конфигурации
    std::string static_dir;               // путь до директории с статическими файлами
}; 

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);