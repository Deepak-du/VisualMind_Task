#include <gtest/gtest.h>
#include "adserver.hpp"
#include <iostream>

// ----------------------------------------------------------------------------
// Test Utility Functions
// ----------------------------------------------------------------------------

TEST(UtilityFunctionsTest, StartsWith) {
    EXPECT_TRUE(starts_with("banner/test", "banner/"));
    EXPECT_FALSE(starts_with("geo", "banner/"));
    EXPECT_TRUE(starts_with("/cfg", "/cfg"));
}

TEST(UtilityFunctionsTest, ReadFile) {
    // Check that file paths are accessible
    std::cout << "Testing with GEO_DATA_PATH: " << GEO_DATA_PATH << std::endl;
    EXPECT_NE(read_file(GEO_DATA_PATH), R"({"error": "File not found"})");
    EXPECT_NE(read_file(PLZ_DATA_PATH), R"({"error": "File not found"})");
    EXPECT_NE(read_file(CONFIG_PATH), R"({"error": "File not found"})");
}

// ----------------------------------------------------------------------------
// Test Request Handlers
// ----------------------------------------------------------------------------

TEST(RequestHandlerTest, HandleRoot) {
    http::response<http::string_body> res;
    handle_root(res);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_TRUE(res.body().find("name") != std::string::npos);
}

TEST(RequestHandlerTest, HandleGeo) {
    http::response<http::string_body> res;
    handle_geo(res);
    EXPECT_EQ(res.result(), http::status::ok);
}

TEST(RequestHandlerTest, HandlePlz) {
    http::response<http::string_body> res;
    handle_plz(res);
    EXPECT_EQ(res.result(), http::status::ok);
}

TEST(RequestHandlerTest, HandleCfg) {
    http::response<http::string_body> res;
    handle_cfg(res);
    EXPECT_EQ(res.result(), http::status::ok);
}

TEST(RequestHandlerTest, HandleNotFound) {
    http::response<http::string_body> res;
    handle_not_found(res);
    EXPECT_EQ(res.result(), http::status::not_found);
}

TEST(RequestHandlerTest, HandleBanner) {
    http::response<http::string_body> res;
    handle_banner("/banner/test", res);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_TRUE(res.body().find("banner") != std::string::npos || res.body().find("error") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Main Test Runner
// ----------------------------------------------------------------------------

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}