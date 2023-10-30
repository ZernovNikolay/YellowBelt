#include "automation.h"

using namespace std::literals;

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"Allowed options"s};

    Args args;

    desc.add_options()
        // Добавляем опцию --help и её короткую версию -h
        ("help,h", "produce help message")
        ("tick-period,t", po::value(&args.period)->value_name("milliseconds"s), "set tick period")
        ("config-file,c", po::value(&args.config)->value_name("file"s), "set config file path")
        ("www-root,w", po::value(&args.static_dir)->value_name("dir"s), "set static files root")
        ("randomize-spawn-points", "spawn dogs at random positions");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        // Если был указан параметр --help, то выводим справку и возвращаем nullopt
        std::cout << desc;
        return std::nullopt;
    }

    // Проверяем наличие опций src и dst
    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config files have not been specified"s);
    }
    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Static file path is not specified"s);
    }
    if(vm.contains("tick-period"s)) {
        args.passive_time_managment = true;
    }
    if(vm.contains("randomize-spawn-points"s)) {
        args.random = true;
    }

    return args;
}