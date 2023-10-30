#include "server_logger.h"

namespace server_logger {

void LoggingRequestHandler::LogResponse(const std::tuple<int, std::string_view>& resp_info) { 
    end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<int, std::milli> diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    Logger::GetInstance().ResponseLog(std::get<int>(resp_info), std::get<std::string_view>(resp_info), diff.count());
}

} // namespace server_logger