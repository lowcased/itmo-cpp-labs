#include <cinttypes>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

struct hash {
	std::size_t operator()(const std::pair<uint64_t, uint64_t>& coords) const {
		std::size_t hash1 = std::hash<uint64_t>()(coords.first);
		std::size_t hash2 = std::hash<uint64_t>()(coords.second);
		return hash1 ^ hash2;
	}
};

uint64_t GenerateRandNum(uint64_t num) {
	std::srand(std::time(0));
	return std::rand() % (num);
}

class Game {
public:
	std::unordered_map<std::pair<uint64_t, uint64_t>, bool, hash> ship;
	Game() {}
	bool& GetMode() {
		return master;
	}
	uint64_t& GetWidth() {
		return width;
	}
	uint64_t& GetHeight() {
		return height;
	}
	__uint128_t& GetKills() {
		return kills;
	}
	__uint128_t& GetDeaths() {
		return deaths;
	}
	uint64_t* GetCount() {
		return count;
	}
	bool& IsWin() {
		return win;
	}
	bool& IsLose() {
		return lose;
	}
	void RestoreGame() {
		deaths = 0;
		kills = 0;
		win = false;
		lose = false;
	}
	bool IsOverlap(uint8_t length, bool horizontal, uint64_t x, uint64_t y) {
		length--;
		uint64_t x_low_limit, x_high_limit;
		if (x == 0) {
			x_low_limit = 0;
		}
		else {
			x_low_limit = x - 1;
		}
		if (x == width - 1) {
			x_high_limit = width - 1;
		}
		else {
			x_high_limit = x + 1;
		}
		uint64_t y_high_limit, y_low_limit;
		if (y == 0) {
			y_low_limit = 0;
		}
		else {
			y_low_limit = y - 1;
		}
		if (y == height - 1) {
			y_high_limit = width - 1;
		}
		else {
			y_high_limit = y + 1;
		}
		if (horizontal) {
			x_high_limit += length;
		}
		else {
			y_high_limit += length;
		}
		if (x_high_limit >= width || y_high_limit >= height) {
			return true;
		}
		for (uint64_t i = y_low_limit; i <= y_high_limit; i++) {
			for (uint64_t j = x_low_limit; j <= x_high_limit; j++) {
				if (i == y_low_limit && j == x_low_limit ||
					i == y_low_limit && j == x_high_limit ||
					i == y_high_limit && j == x_low_limit ||
					i == y_high_limit && j == x_high_limit) {
					if (ship[{j, i}]) {
						return true;
					}
				}
			}
		}
		return false;
	}
	void SetShips() {
		for (uint8_t i = 4; i > 0; i--) {
			uint64_t left = count[i];
			while (left > 0) {
				uint64_t x = GenerateRandNum(width);
				uint64_t y = GenerateRandNum(height);
				bool horizontal = GenerateRandNum(2);
				if (!IsOverlap(i, horizontal, x, y)) {
					if (horizontal) {
						for (uint64_t j = x; j < x + i; j++) {
							ship[{j, y}] = true;
						}
					}
					else {
						for (uint64_t j = y; j < y + i; j++) {
							ship[{x, j}] = true;
						}
					}
					left--;
				}
			}
		}
	}

private:
	bool master = true;
	uint64_t width = 1;
	uint64_t height = 1;
	uint64_t count[4]{ 0, 0, 0, 0 };
	__uint128_t deaths = 0;
	__uint128_t kills = 0;
	bool win = false;
	bool lose = false;
};

class Shot {
public:
	bool custom = true;
	bool hit1_flag = false;
	bool hit2_flag = false;
	char shot_dir = 'u';
	std::pair<uint64_t, uint64_t> hit{ 0, 0 };
	Shot() {}
	std::pair<uint64_t, uint64_t> GetShot(uint64_t& width, uint64_t& height) {
		if (custom) {
			return CustomShot(width, height);
		}
		else {
			return OrderedShot(width, height);
		}
	}

private:
	std::pair<uint64_t, uint64_t> order{ 0, 0 };
	std::pair<uint64_t, uint64_t> CustomShot(uint64_t& width, uint64_t& height) {
		uint64_t shotx, shoty;
		if (hit1_flag) {
			if (hit2_flag) {
				if (shot_dir == 'u') {
					return { hit.first, hit.second-- };
				}
				if (shot_dir == 'd') {
					return { hit.first, hit.second++ };
				}
				if (shot_dir == 'l') {
					return { hit.first--, hit.second };
				}
				if (shot_dir == 'r') {
					return { hit.first++, hit.second };
				}
			}
			else {
				if (shot_dir == 'u') {
					if (hit.second == 0) {
						shot_dir = 'd';
					}
					else {
						shotx = hit.first;
						shoty = hit.second--;
						return { shotx, shoty };
					}
				}
				if (shot_dir == 'd') {
					if (hit.second == height - 1) {
						shot_dir = 'l';
					}
					else {
						shotx = hit.first;
						shoty = hit.second++;
						return { shotx, shoty };
					}
				}
				if (shot_dir == 'l') {
					if (hit.second == 0) {
						shot_dir = 'r';
					}
					else {
						shotx = hit.first--;
						shoty = hit.second;
						return { shotx, shoty };
					}
				}
				if (shot_dir == 'r') {
					return { shotx + 1, shoty };
				}
			}
		}
		else {
			shotx = GenerateRandNum(width);
			shoty = GenerateRandNum(height);
			hit = { shotx, shoty };
		}
		return { shotx, shoty };
	}
	std::pair<uint64_t, uint64_t> OrderedShot(uint64_t& width, uint64_t& height) {
		if (order.first == width - 1) {
			if (order.second != height - 1) {
				order.first = 0;
				order.second++;
			}
		}
		return order;
	}
};

void SaveGame(Game* game, std::string path) {
	std::ofstream out;
	out.open(path);
	out << game->GetWidth() << " " << game->GetHeight() << "\n";
	for (const std::pair<const std::pair<uint64_t, uint64_t>, bool>& coords : game->ship) {
		if (coords.first.first != (uint64_t)0) {
			if (game->ship.find({ coords.first.first - 1, coords.first.second }) != game->ship.end()) {
				continue;
			}
		}
		if (coords.first.second != (uint64_t)0) {
			if (game->ship.find({ coords.first.first, coords.first.second - 1 }) != game->ship.end()) {
				continue;
			}
		}
		uint8_t length = 1;
		char direction = 'v';
		if (game->ship.find({ coords.first.first + 1, coords.first.second }) != game->ship.end()) {
			direction = 'h';
			length++;
			if (game->ship.find({ coords.first.first + 2, coords.first.second }) != game->ship.end()) {
				length++;
				if (game->ship.find({ coords.first.first + 3, coords.first.second }) != game->ship.end()) {
					length++;
				}
			}
		}
		else if (game->ship.find({ coords.first.first, coords.first.second + 1 }) != game->ship.end()) {
			length++;
			if (game->ship.find({ coords.first.first, coords.first.second + 2 }) != game->ship.end()) {
				length++;
				if (game->ship.find({ coords.first.first, coords.first.second + 3 }) != game->ship.end()) {
					length++;
				}
			}
		}
		out << static_cast<uint16_t>(length) << " " << direction << " " << coords.first.first << " " << coords.first.second << "\n";
	}
	out.close();
}

void LoadGame(Game* game, std::string path) {
	std::ifstream in;
	in.open(path);
	in >> game->GetWidth() >> game->GetHeight();
	char length;
	char direction;
	uint64_t x, y;
	while (in >> length >> direction >> x >> y) {
		if (direction == 'h') {
			for (uint8_t i = 0; i <= length - '1'; i++) {
				game->ship[{x + i, y}] = true;
			}
		}
		else {
			for (uint8_t i = 0; i <= length - '1'; i++) {
				game->ship[{x, y + i}] = true;
			}
		}
		game->GetCount()[length - '1']++;
	}
	in.close();
}

int main() {
	Game* game = new Game;
	Shot* shot = new Shot;
	while (true) {
		std::string cmd;
		std::getline(std::cin, cmd);
		if (cmd == "exit") {
			std::cout << "ok\n";
			return 0;
		}
		if (cmd == "ping") {
			std::cout << "pong\n";
		}
		else if (cmd.substr(0, 6) == "create") {
			if (cmd.substr(7) == "master") {
				game->GetMode() = true;
				std::cout << "ok\n";
			}
			else if (cmd.substr(7) == "slave") {
				game->GetMode() = false;
				std::cout << "ok\n";
			}
		}
		else if (cmd == "start") {
			game->SetShips();
			std::cout << "ok\n";
		}
		else if (cmd == "stop") {
			game->RestoreGame();
			std::cout << "ok\n";
		}
		else if (cmd.substr(0, 9) == "set width") {
			game->GetWidth() = std::stoull(cmd.substr(10));
			std::cout << "ok\n";
		}
		else if (cmd == "get width") {
			std::cout << game->GetWidth() << "\n";
		}
		else if (cmd.substr(0, 10) == "set height") {
			game->GetHeight() = std::stoull(cmd.substr(11));
			std::cout << "ok\n";
		}
		else if (cmd == "get height") {
			std::cout << game->GetHeight() << "\n";
		}
		else if (cmd.substr(0, 9) == "set count") {
			int idx = std::stoi(cmd.substr(10, 1)) - 1;
			game->GetCount()[idx] = std::stoull(cmd.substr(12));
			std::cout << "ok\n";
		}
		else if (cmd.substr(0, 9) == "get count") {
			std::cout << game->GetCount()[std::stoi(cmd.substr(10)) - 1] << "\n";
		}
		else if (cmd.substr(0, 12) == "set strategy") {
			if (cmd.substr(0, 13) == "ordered") {
				shot->custom = false;
				shot->hit1_flag = false;
				shot->hit2_flag = false;
			}
			else {
				shot->custom = true;
			}
			std::cout << "ok\n";
		}
		else if (cmd.substr(0, 4) == "shot") {
			if (cmd.length() == 4) {
			}
			else {
				size_t space_pos = cmd.find(' ', 5);
				uint64_t x = std::stoull(cmd.substr(5, space_pos - 5));
				uint64_t y = std::stoull(cmd.substr(space_pos + 1));
				if (game->ship[{x, y}]) {
					game->ship[{x, y}] = false;
					uint64_t x1 = x, y1 = y;
					if (x != 0) {
						x1--;
					}
					if (y != 0) {
						y1--;
					}
					if (game->ship[{x1, y}] || game->ship[{x, y1}] || game->ship[{x + 1, y}] || game->ship[{x, y + 1}]) {
						std::cout << "hit\n";
					}
					else {
						std::cout << "kill\n";
						game->GetDeaths()++;
						if (!game->IsWin()) {
							uint64_t* ptr = game->GetCount();
							__uint128_t sum = ptr[0] + ptr[1] + ptr[2] + ptr[3];
							if (game->GetDeaths() == sum) {
								game->IsLose() = true;
							}
						}
					}
				}
				else {
					std::cout << "miss\n";
				}
			}
		}
		else if (cmd.substr(0, 10) == "set result") {
			std::string result = cmd.substr(11);
			if (result == "hit" && shot->custom) {
				if (!shot->hit1_flag) {
					shot->hit1_flag = true;
				}
				else if (!shot->hit2_flag) {
					shot->hit2_flag = true;
				}
			}
			else if (result == "kill") {
				game->GetKills()++;
				shot->hit1_flag = false;
				shot->hit2_flag = false;
				shot->shot_dir = 'u';
				if (!game->IsLose()) {
					uint64_t* ptr = game->GetCount();
					__uint128_t sum = ptr[0] + ptr[1] + ptr[2] + ptr[3];
					if (game->GetKills() == sum) {
						game->IsWin() = true;
					}
				}
			}
			std::cout << "ok\n";
		}

		else if (cmd == "finished") {
			if (game->IsWin() || game->IsLose()) {
				std::cout << "yes\n";
			}
			else {
				std::cout << "no\n";
			}
		}
		else if (cmd == "win") {
			if (game->IsWin()) {
				std::cout << "yes\n";
			}
			else {
				std::cout << "no\n";
			}
		}
		else if (cmd == "lose") {
			if (game->IsLose()) {
				std::cout << "yes\n";
			}
			else {
				std::cout << "no\n";
			}
		}
		else if (cmd.substr(0, 4) == "dump") {
			SaveGame(game, cmd.substr(5));
			std::cout << "ok\n";
		}
		else if (cmd.substr(0, 4) == "load") {
			LoadGame(game, cmd.substr(5));
			std::cout << "ok\n";
		}
	}
}
