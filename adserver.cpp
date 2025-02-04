#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <thread>
#include <array>
#include <string_view>
#include "adserver.hpp"

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
namespace json  = boost::json;
using tcp       = asio::ip::tcp;

// -----------------------------------------------------------------------------
// Helper: C++17 version of starts_with for std::string_view
// -----------------------------------------------------------------------------
bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// -----------------------------------------------------------------------------
// Global constants and environment variables
// -----------------------------------------------------------------------------

const std::string pod_name = [] {
    if (const char* p = std::getenv("POD_NAME"))
        return std::string(p);
    else
        return std::string("unknown-pod");
}();

const std::string pod_namespace = [] {
    if (const char* p = std::getenv("POD_NAMESPACE"))
        return std::string(p);
    else
        return std::string("unknown-namespace");
}();

// -----------------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------------

std::string read_file(std::string_view path) {
    std::ifstream file{std::string(path)};
    if (!file)
        return R"({"error": "File not found"})";
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string fetch_banner(std::string_view banner_name) {
    std::string db_url  = "http://db-svc/get?k=banner-" + std::string(banner_name);
    std::string command = "curl -s --max-time 5 " + db_url;
    std::string result;
    std::array<char, 128> buffer{};
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        return "{}";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        result.append(buffer.data());
    pclose(pipe);
    if (result.empty())
        return R"({"error":"Couldn't get the banner"})";
    else
        return "{\"banner\":\"" + result + "\"}";
}

// -----------------------------------------------------------------------------
// Request Handlers
// -----------------------------------------------------------------------------

void handle_root(http::response<http::string_body>& res) {
    json::object obj;
    obj["name"]      = pod_name;
    obj["namespace"] = pod_namespace;
    res.result(http::status::ok);
    res.body() = json::serialize(obj);
}

void handle_banner(std::string_view target, http::response<http::string_body>& res) {
    std::string banner_name(target.substr(8));
    res.result(http::status::ok);
    res.body() = fetch_banner(banner_name);
}

void handle_geo(http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.body() = read_file(GEO_DATA_PATH);
}

void handle_plz(http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.body() = read_file(PLZ_DATA_PATH);
}

void handle_cfg(http::response<http::string_body>& res) {
    res.result(http::status::ok);
    res.body() = read_file(CONFIG_PATH);
}

void handle_not_found(http::response<http::string_body>& res) {
    res.result(http::status::not_found);
    res.body() = R"({"error":"Not found"})";
}

void handle_request(http::request<http::string_body>& req, http::response<http::string_body>& res) {
    std::string_view target = req.target();
    if (target == "/") {
        handle_root(res);
    }
    else if (starts_with(target, "/banner")) {
        handle_banner(target, res);
    }
    else if (target == "/geo") {
        handle_geo(res);
    }
    else if (target == "/plz") {
        handle_plz(res);
    }
    else if (target == "/cfg") {
        handle_cfg(res);
    }
    else {
        handle_not_found(res);
    }
    res.set(http::field::content_type, "application/json");
    res.prepare_payload();
}

void session(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);
        http::response<http::string_body> res;
        handle_request(req, res);
        http::write(socket, res);
    }
    catch (const std::exception& e) {
        std::cerr << "Session error: " << e.what() << "\n";
    }
}

void run_server(asio::io_context& io_context, unsigned short port) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::thread(session, std::move(socket)).detach();
    }
}

#ifndef UNIT_TESTING  // Exclude main() when running unit tests
int main() {
    try {
        asio::io_context io_context;
        run_server(io_context, 80);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
    }
    return 0;
}
#endif