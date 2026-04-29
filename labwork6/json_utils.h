#pragma once
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

void SaveJSON(cpr::Response &response, std::string &date,
              const std::string &from_code, const std::string &to_code);

bool LoadJSON(nlohmann::json &ans, std::string &date,
              const std::string &from_code, const std::string &to_code);