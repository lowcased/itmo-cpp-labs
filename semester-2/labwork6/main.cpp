#include "json_utils.h"
#include "races.h"
#include <Windows.h>
#include <fstream>
#include <iostream>

void GetAPI(std::string &apikey) {
  std::ifstream ifile("api key.txt");
  if (ifile.is_open()) {
    ifile >> apikey;
    ifile.close();
    return;
  }
  std::cout << "Yandex API-key: ";
  std::cin >> apikey;
  std::ofstream ofile("api key.txt");
  ofile << apikey;
  ofile.close();
}

int main() {
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  std::string apikey;
  GetAPI(apikey);
  while (true) {
    std::string date;
    std::cout << "Date (YYYY-MM-DD): ";
    std::cin >> date;
    if (date == "stop")
      return 0;
    const std::string spb_code = "c2";
    const std::string pskov_code = "c25";
    std::cout << "\n===========================\n  Санкт-Петербург - "
                 "Псков\n===========================\n";
    GetRaces(apikey, date, spb_code, pskov_code);
    std::cout << "\n===========================\n  Псков - "
                 "Санкт-Петербург\n===========================\n";
    GetRaces(apikey, date, pskov_code, spb_code);
  }
}