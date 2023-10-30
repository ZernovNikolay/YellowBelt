#include "request_handler.h"

namespace http_handler {

std::string DecodeURI (const std::string& URI) {
    std::string ret;
    char ch;
    int ii = 0;
    for (size_t i=0; i<URI.length(); i++) {
        if (URI[i]=='%') {
            sscanf(URI.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=URI[i];
        }
    }
    return (ret);
}

fs::path RequestHandler::CheckIsSubPath(const std::string_view& target) {

    fs::path full_path = base_path_;
    full_path += target;
    full_path = fs::weakly_canonical(full_path);

    // Приводим оба пути к каноничному виду (без . и ..)
    full_path = fs::weakly_canonical(full_path);
    base_path_ = fs::weakly_canonical(base_path_);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base_path_.begin(), p = full_path.begin(); b != base_path_.end(); ++b, ++p) {
        if (p == full_path.end() || *p != *b) {
            throw RequestException(ErrorType::BAD_REQUEST);
        }
    }

    return full_path;
}


void RequestHandler::CheckEntryPoint(fs::path& full_path) {

    if (!full_path.has_extension()) {

        if(full_path != base_path_) {
            throw RequestException(ErrorType::BAD_REQUEST); 
        }else{
            full_path /= "index.html";
        }
    }

}

FileResponse RequestHandler::CheckDownloadFile(const fs::path& full_path, RequestInfo& info_req) {

    boost::beast::http::file_body::value_type file;
    sys::error_code ec;

    file.open(full_path.c_str(), beast::file_mode::read, ec);
    

    if(ec) {

        throw RequestException(ErrorType::FILE_NOT_FOUND);

    }else{

        std::string str = full_path.extension().c_str();

        if(auto it = ContentType::Extension_type.find(boost::algorithm::to_lower_copy(str)); it == ContentType::Extension_type.end()) {
            return MakeFileResponse(http::status::ok, std::move(file), info_req, ContentType::UNKNOWN_TYPE);
        }else{
            return MakeFileResponse(http::status::ok, std::move(file), info_req, it->second);
        }
    }

}

FileResponse RequestHandler::HandleFileRequest(RequestInfo& info_req) {

    info_req.method_allowed = RequestMethod::ALLOW_GET_HEAD;
    CheckMethod(info_req.method_allowed, info_req.method);

    fs::path full_path = CheckIsSubPath(info_req.target);
    CheckEntryPoint(full_path);
    return CheckDownloadFile(full_path, info_req);
    
}

}  // namespace http_handler
