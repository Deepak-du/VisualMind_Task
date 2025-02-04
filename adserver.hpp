#pragma once
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <string>

// Use namespaces
namespace http = boost::beast::http;
namespace json = boost::json;

// Define global constants (Ensure they match adserver.cpp)
inline const std::string_view GEO_DATA_PATH = "/mnt/adserver_geodata/geodata.txt";
inline const std::string_view PLZ_DATA_PATH = "/mnt/adserver_geodata/plz.txt";
inline const std::string_view CONFIG_PATH   = "/mnt/adserver_farm_base/adserver-config.conf";

// Function declarations (from adserver.cpp)
bool starts_with(std::string_view s, std::string_view prefix);
std::string read_file(std::string_view path);
std::string fetch_banner(std::string_view banner_name);
void handle_root(http::response<http::string_body>& res);
void handle_banner(std::string_view target, http::response<http::string_body>& res);
void handle_geo(http::response<http::string_body>& res);
void handle_plz(http::response<http::string_body>& res);
void handle_cfg(http::response<http::string_body>& res);
void handle_not_found(http::response<http::string_body>& res);