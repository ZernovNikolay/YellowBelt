#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "server_logger.h"
#include "automation.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace 

int main(int argc, const char* argv[]) {
    try {

        auto args = ParseCommandLine(argc, argv);

        InitBoostLogFilter();

        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(args->config);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
                Logger::GetInstance().StopServerLog(ec.value(), ""sv);
            }
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        std::filesystem::path base_path(args->static_dir);
        base_path = std::filesystem::weakly_canonical(base_path);

        //создаём базу игроков

        std::shared_ptr<game_unit::PlayerLibrary> players = std::make_shared<game_unit::PlayerLibrary>();
        std::shared_ptr<game_unit::Tokenizer> tokenizer = std::make_shared<game_unit::Tokenizer>();
        std::shared_ptr<http_handler::GameDataBaseHolder> database_holder = std::make_shared<http_handler::GameDataBaseHolder>(std::move(game), players, tokenizer);

        //создаём таймер

        auto api_strand = net::make_strand(ioc);
        std::mutex api_request_lock;
        http_handler::Application app{database_holder};
        auto time = std::chrono::milliseconds(args->period);

        auto ticker = std::make_shared<passive_update::Ticker>(api_request_lock, api_strand, time,
            [&app](std::chrono::milliseconds delta) { app.Tick(delta); }
        );

        // решаем, активен таймер или нет

        database_holder->SetStartParameters(args->passive_time_managment, args->random);

        if(args->passive_time_managment) {
            ticker->Start();
           
        }

        // создаём handler'ы для ответов 

        std::shared_ptr<http_handler::RequestHandler> handler = std::make_shared<http_handler::RequestHandler>(database_holder, std::move(base_path), api_strand, api_request_lock);
        server_logger::LoggingRequestHandler logging_handler{handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов     
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](const std::string ip, auto&& req, auto&& send) {

            logging_handler(ip, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));

        });

        Logger::GetInstance().StartServerLog(port, address.to_string());

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });



    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;

        Logger::GetInstance().StopServerLog(EXIT_FAILURE, ex.what());

        return EXIT_FAILURE;
    }
}