#pragma once
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

void PrintRaces(nlohmann::json &ans);

void GetRaces(std::string &apikey, std::string &date,
              const std::string &from_code, const std::string &to_code);
