#include "json_utils.h"

void SaveJSON(cpr::Response &response, std::string &date,
              const std::string &from_code, const std::string &to_code) {
  std::ofstream file(date + " " + from_code + "-" + to_code + ".json");
  std::time_t cur_time = std::time(0);
  std::tm time = *std::localtime(&cur_time);
  file << time.tm_hour << ":" << time.tm_mday << ":" << time.tm_mon + 1 << ":"
       << time.tm_year + 1900 << "\n";
  file << response.text;
  file.close();
}

bool LoadJSON(nlohmann::json &ans, std::string &date,
              const std::string &from_code, const std::string &to_code) {
  std::ifstream file(date + " " + from_code + "-" + to_code + ".json");
  if (file.is_open()) {
    std::string file_date;
    std::getline(file, file_date);
    std::istringstream date_stream(file_date);
    char delimiter;
    int hour, day, month, year;
    date_stream >> hour >> delimiter >> day >> delimiter >> month >>
        delimiter >> year;
    std::time_t cur_time = std::time(0);
    std::tm time = *std::localtime(&cur_time);
    if (year + 1900 != time.tm_year || month + 1 != time.tm_mon ||
        day != time.tm_mday || hour != time.tm_hour) {
      file.close();
      return false;
    }
    file >> ans;
    file.close();
    return true;
  }
  return false;
}