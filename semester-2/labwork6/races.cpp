#include "races.h"
#include "json_utils.h"

void PrintRaces(nlohmann::json &ans) {
    const nlohmann::json_abi_v3_11_2::json &segments = ans["segments"];
    for (const nlohmann::json_abi_v3_11_2::json &segment : segments) {
      if (segment["has_transfers"] == false) {
        std::cout << "Место отправления: "
                  << segment["from"]["title"].get<std::string>() << "\n"
                  << "Время отправления: "
                  << segment["departure"].get<std::string>() << "\n"
                  << "Транспорт: "
                  << segment["from"]["transport_type"].get<std::string>()
                  << "\n\n";
        std::cout << "Место прибытия: "
                  << segment["to"]["title"].get<std::string>() << "\n"
                  << "Время прибытия: " << segment["arrival"].get<std::string>()
                  << "\n\n\n";
      } else if (segment["transfers"].size() == 1) {
        std::cout
            << "Место отправления: "
            << segment["departure_from"]["title"].get<std::string>() << "\n"
            << "Время отправления: " << segment["departure"].get<std::string>()
            << "\n"
            << "Транспорт: "
            << segment["departure_from"]["transport_type"].get<std::string>()
            << "\n\n";
        std::cout << "Место пересадки: "
                  << segment["transfers"][0]["title"].get<std::string>() << "\n"
                  << "Время прибытия: "
                  << segment["details"][0]["arrival"].get<std::string>() << "\n"
                  << "Время отправления: "
                  << segment["details"][2]["departure"].get<std::string>() << "\n"
                  << "Транспорт: "
                  << segment["details"][2]["thread"]["transport_type"] << "\n\n";
        std::cout << "Место прибытия: "
                  << segment["arrival_to"]["title"].get<std::string>() << "\n"
                  << "Время прибытия: " << segment["arrival"].get<std::string>()
                  << "\n\n\n";
      }
    }
  }

void GetRaces(std::string &apikey, std::string &date,
              const std::string &from_code, const std::string &to_code) {
  nlohmann::json ans;
  if (!LoadJSON(ans, date, from_code, to_code)) {
    cpr::Parameters params = {{"apikey", apikey},
                              {"from", from_code},
                              {"to", to_code},
                              {"date", date},
                              {"transfers", "true"}};
    cpr::Response response =
        cpr::Get(cpr::Url{"https://api.rasp.yandex.net/v3.0/search/"}, params);
    ans = nlohmann::json::parse(response.text);
    SaveJSON(response, date, from_code, to_code);
  }
  PrintRaces(ans);
}