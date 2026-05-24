#include <iostream>
#include <fstream>


struct ErrorStruct {
	char* url = new char[1000];
	long long countURL = 0;
};

enum class Months {
	Jan = 1,
	Feb,
	Mar,
	Apr,
	May,
	Jun,
	Jul,
	Aug,
	Sep,
	Oct,
	Nov,
	Dec
};

void PrintError(int errorID) {
	if (errorID == 1) {
		std::cout << "\nLog file not found";
	}
	else if (errorID == 2) {
		std::cout << "\nInvalid argument entered";
	}
	else if (errorID == 3) {
		std::cout << "\nNot enough arguments entered";
	}
}

char* Substring(char* s, int l, int r) {
	char* substr = new char[r - l + 1];
	for (int i = l; i < r; ++i) {
		substr[i - l] = s[i];
	}
	substr[r - l] = '\0';
	return substr;
}

Months GetMonth(char buf[5]) {
	Months month;
	if (buf[0] == 'J') {
		if (buf[1] == 'a') {
			if (buf[2] == 'n') {
				month = Months::Jan;
			}
		}
		else if (buf[1] == 'u') {
			if (buf[2] == 'n') {
				month = Months::Jun;
			}
			else if (buf[2] == 'l') {
				month = Months::Jul;
			}
		}
	}
	else if (buf[0] == 'F') {
		if (buf[1] == 'e') {
			if (buf[2] == 'b') {
				month = Months::Feb;
			}
		}
	}
	else if (buf[0] == 'M') {
		if (buf[1] == 'a') {
			if (buf[2] == 'r') {
				month = Months::Mar;
			}
			else if (buf[2] == 'y') {
				month = Months::May;
			}
		}
	}
	else if (buf[0] == 'A') {
		if (buf[1] == 'p') {
			if (buf[2] == 'r') {
				month = Months::Apr;
			}
		}
		else if (buf[1] == 'u') {
			if (buf[2] == 'g') {
				month = Months::Aug;
			}
		}
	}
	else if (buf[0] == 'S') {
		if (buf[1] == 'e') {
			if (buf[2] == 'p') {
				month = Months::Sep;
			}
		}
	}
	else if (buf[0] == 'O') {
		if (buf[1] == 'c') {
			if (buf[2] == 't') {
				month = Months::Oct;
			}
		}
	}
	else if (buf[0] == 'N') {
		if (buf[1] == 'o') {
			if (buf[2] == 'v') {
				month = Months::Nov;
			}
		}
	}
	else if (buf[0] == 'D') {
		if (buf[1] == 'e') {
			if (buf[2] == 'c') {
				month = Months::Dec;
			}
		}
	}
	return month;
}

int calculateLeapYears(int year)
{
	return (year / 4) - (year / 100) + (year / 400);
}

int FindLeapYearsBetween(int l, int r)
{
	int num1 = calculateLeapYears(r);
	int num2 = calculateLeapYears(l - 1);
	return num1 - num2;
}

long long ConvertDateToTimestamp(char* date) {
	int day, year, hours, minutes, sec;
	char buf[5];
	int i = 0, count = 0;
	for (; i < 2; i++) {
		buf[count] = date[i];
		count++;
	}
	day = (buf[0] - '0') * 10 + (buf[1] - '0') - 1;
	i++;
	count = 0;
	for (; count < 3; ++i) {
		buf[count] = date[i];
		count++;
	}
	Months month = GetMonth(buf);
	i++;
	count = 0;
	for (; count < 4; ++i) {
		buf[count] = date[i];
		count++;
	}
	year = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0');
	switch ((Months)month) {
	case Months::Dec:
		day += 30;
	case Months::Nov:
		day += 31;
	case Months::Oct:
		day += 30;
	case Months::Sep:
		day += 31;
	case Months::Aug:
		day += 31;
	case Months::Jul:
		day += 30;
	case Months::Jun:
		day += 31;
	case Months::May:
		day += 30;
	case Months::Apr:
		day += 31;
	case Months::Mar:
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
			day += 29;
		}
		else {
			day += 28;
		}
	case Months::Feb:
		day += 31;
	}
	const int unixEpoch = 1970;
	const int daysInYear = 365;
	day += FindLeapYearsBetween(unixEpoch, year);
	year -= unixEpoch;
	day += year * daysInYear;
	i++;
	count = 0;
	for (; count < 2; ++i) {
		buf[count] = date[i];
		count++;
	}
	hours = (buf[0] - '0') * 10 + (buf[1] - '0');
	i++;
	count = 0;
	for (; count < 2; ++i) {
		buf[count] = date[i];
		count++;
	}
	minutes = (buf[0] - '0') * 10 + (buf[1] - '0');
	i++;
	count = 0;
	for (; count < 2; ++i) {
		buf[count] = date[i];
		count++;
	}
	sec = (buf[0] - '0') * 10 + (buf[1] - '0');
	count = 0;
	i++;
	for (; count < 5; ++i) {
		buf[count] = date[i];
		count++;
	}
	int hourOffset = (buf[1] - '0') * 10 + (buf[2] - '0');
	int minutesOffset = (buf[3] - '0') * 10 + (buf[4] - '0');
	if (buf[0] == '+') {
		hours -= hourOffset;
		minutes -= minutesOffset;
	}
	else {
		hours += hourOffset;
		minutes += minutesOffset;
	}
	long long time = day * 86400 + hours * 3600 + minutes * 60 + sec;
	return time;
}


long long OutputErrors(char* log, char* path, bool print, long long from, long long to) {
	std::ifstream input(log);
	std::ofstream output(path);
	long long errorCount = 0;
	if (input.is_open() && output.is_open()) {
		char* request;
		if (print) {
			std::cout << "\nErrors:\n";
		}
		while (!input.eof()) {
			request = new char[1000];
			int count = 0;
			char c;
			while ((c = input.get()) != '\n' && c != EOF) {
				request[count] = c;
				count++;
			}
			char* date = new char[count];
			int idx = -1;
			for (int i = 0; i < count; i++) {
				if (request[i] == '[') {
					idx = 0;
					continue;
				}
				if (idx != -1) {
					if (request[i] == ']') {
						break;
					}
					else {
						date[idx] = request[i];
						idx++;
					}
				}
			}
			if (ConvertDateToTimestamp(date) >= from && ConvertDateToTimestamp(date) <= to) {
				int i=count-1;
				for (; i >= 4; i--) {
					if (request[i] == ' ') {
						break;
					}
				}
				if (request[i - 3] == '5') {
					output << request << "\n";
					errorCount++;
					if (print) {
						std::cout << request << "\n";
					}
				}
			}
			delete(request, date);
		}
	}
	return errorCount;
}


int FindError(char* url, int count, ErrorStruct* error) {
	for (int i = 0; i < count; i++) {
		if (strcmp(error[i].url, url)==0) {
			return i;
		}
	}
	return count;
}


void ErrorSort(ErrorStruct* error, long long count) {
	long long i, j;
	ErrorStruct key;
	for (i = 1; i < count; i++) {
		key = error[i];
		j = i - 1;
		while (j >= 0 && error[j].countURL > key.countURL) {
			error[j + 1] = error[j];
			j = j - 1;
		}
		error[j + 1] = key;
	}
}


void ErrorStats(char* log, int n, char* path, long long errorCount) {
	std::cout << errorCount << " " << n;
	ErrorStruct* error = new ErrorStruct[errorCount];
	std::ifstream input(path);
	long long count = 0;
	if (input.is_open()) {
		while (!input.eof()) {
			char c;
			int length = 0;
			char* url = new char[1000];
			c = input.get();
			while (c != '\n' && c != EOF && c != '"') {
				c = input.get();
			}
			if (c == '"') {
				c = input.get();
				while (c != '\n' && c != EOF && c != '"') {
					c = input.get();
					url[length] = c;
					length++;
				}
				url[length] = '\0';
			}
			int idx = FindError(url, count, error);
			if (idx == count) {
				count++;
				error[idx].url = url;
			}
			error[idx].countURL++;
			delete(url);
		}
		ErrorSort(error, count);
		std::cout << "\nStats:\n";
		for (int i = 0; i < n; i++) {
			std::cout << error[i].url << "\n";
		}
	}

	delete(&error);
	input.close();
}


void MaxRequestsInWindow(char* log, long long timeInterval, long long from, long long to) {
	std::ifstream input1(log);
	std::ifstream input2(log);
	long long result = 1;
	long long begin_pointer = 0;
	if (input1.is_open() && input2.is_open()) {
		long long time1 = 0;
		long long time2 = 0;
		long long pointer1 = 0;
		long long pointer2 = 0;
		while (!input2.eof()) {
			char* request2 = new char[1000];
			int count2 = 0;
			char c2;
			while ((c2 = input2.get()) != '\n' && c2 != EOF) {
				request2[count2] = c2;
				count2++;
			}
			char* date = new char[count2];
			int idx = -1;
			for (int i = 0; i < count2; i++) {
				if (request2[i] == '[') {
					idx = 0;
					continue;
				}
				if (idx != -1) {
					if (request2[i] == ']') {
						break;
					}
					else {
						date[idx] = request2[i];
						idx++;
					}
				}
			}
			pointer2++;
			if (idx > 0 && ConvertDateToTimestamp(date) >= from) {
				time2 = ConvertDateToTimestamp(date);
				time1 = ConvertDateToTimestamp(date);
				break;
			}
			delete(request2, date);
		}
		while (pointer1 < pointer2) {
			char c1;
			while ((c1 = input1.get()) != '\n' && c1 != EOF);
			pointer1++;
		}
		while (!input2.eof() && time2 <= to) {
			while (!input2.eof() && time2 - time1 <= timeInterval) {
				char* request2 = new char[1000];
				int count2 = 0;
				char c2;
				while ((c2 = input2.get()) != '\n' && c2 != EOF) {
					request2[count2] = c2;
					count2++;
				}
				char* date = new char[count2];
				int idx = -1;
				for (int i = 0; i < count2; i++) {
					if (request2[i] == '[') {
						idx = 0;
						continue;
					}
					if (idx != -1) {
						if (request2[i] == ']') {
							break;
						}
						else {
							date[idx] = request2[i];
							idx++;
						}
					}
				}
				pointer2++;
				if (idx > 0) {
					time2 = ConvertDateToTimestamp(date);
				}
				delete(request2, date);
			}

			if (result < pointer2 - pointer1) {
				result = pointer2 - pointer1;
				begin_pointer = pointer1;
			}

			while (time2 - time1 > timeInterval) {
				char* request1 = new char[1000];
				int count1 = 0;
				char c1;
				while ((c1 = input1.get()) != '\n' && c1 != EOF) {
					request1[count1] = c1;
					count1++;
				}
				char* date = new char[count1];
				int idx = -1;
				for (int i = 0; i < count1; i++) {
					if (request1[i] == '[') {
						idx = 0;
						continue;
					}
					if (idx != -1) {
						if (request1[i] == ']') {
							break;
						}
						else {
							date[idx] = request1[i];
							idx++;
						}
					}
				}
				pointer1++;
				if (idx > 0) {
					time1 = ConvertDateToTimestamp(date);
				}
				delete(request1, date);
			}
		}
	}
	std::ifstream input(log);
	if (input.is_open()) {
		long long pointer = 0;
		begin_pointer--;
		while (pointer < begin_pointer) {
			char c;
			while ((c = input.get()) != '\n');
			pointer++;
		}
		while (pointer - begin_pointer < result) {
			char* request = new char[1000];
			long long length = 0;
			char c;
			while ((c = input.get()) != '\n' && c != EOF) {
				request[length] = c;
				length++;
			}
			request[length] = '\0';
			std::cout << request << "\n";
			pointer++;
			delete(request);
		}
	}
}


bool IsNumber(char* str) {
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] != '0' && str[i] != '1' &&
			str[i] != '2' && str[i] != '3' &&
			str[i] != '4' && str[i] != '5' &&
			str[i] != '6' && str[i] != '7' &&
			str[i] != '8' && str[i] != '9') {
			return false;
		}
	}
	return true;
}


int ParseArguments(int argc, char** argv, char* log) {
	long long from = 0, to = LLONG_MAX;
	bool print = false, output = false;
	char* path = (char*)"";
	long long errorCount;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-f")==0 || strcmp(argv[i], "--from")==0) {
			if (i < argc - 1) {
				from = atoi(argv[i + 1]);
				i++;
				continue;
			}
		}
		else if (strncmp(argv[i], "--from=", 7)==0) {
			from = atoi(Substring(argv[i], 7, strlen(argv[i])));
		}

		else if (strcmp(argv[i], "-t")==0 || strcmp(argv[i], "--to")==0) {
			if (i < argc - 1) {
				to = atoi(argv[i + 1]);
				i++;
				continue;
			}
		}
		else if (strncmp(argv[i], "--to=", 5)==0) {
			to = atoi(Substring(argv[i], 5, strlen(argv[i])));
		}

		else if (strcmp(argv[i], "-p")==0 || strcmp(argv[i], "--print")==0) {
			print = true;
		}

		else if (strcmp(argv[i], "-o")==0 || strcmp(argv[i], "--output")==0 || strncmp(argv[i], "--output=", 9)==0) {
			output = true;
			if (strcmp(argv[i], "-o")==0 || strcmp(argv[i], "--output")==0) {
				if (i < argc - 1) {
					path = argv[i + 1];
				}
				i++;
				continue;
			}
			else {
				path = Substring(argv[i], 9, strlen(argv[i]));
			}
		}
	}
	if (output) {
		errorCount = OutputErrors(log, path, print, from, to);
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], log)!=0) {
			if (strcmp(argv[i], "-s")==0 || strcmp(argv[i], "--stats")==0 || strncmp(argv[i], "--stats=",8)==0) {
				long long n;
				if (strcmp(argv[i], "-s")==0 || strcmp(argv[i], "--stats")==0) {
					if (i < argc - 1) {
						if (IsNumber(argv[i + 1])) {
							if (output) {
								n = atoi(argv[i + 1]);
								ErrorStats(log, n, path, errorCount);
							}
							i++;
							continue;
						}
						else if (output) {
							n = 10;
							ErrorStats(log, n, path, errorCount);
						}
					}
					else if (output) {
						n = 10;
						ErrorStats(log, n, path, errorCount);
					}
				}
				else if (output && IsNumber(Substring(argv[i], 8, strlen(argv[i])))) {
					n = atoi(Substring(argv[i], 8, strlen(argv[i])));
					ErrorStats(log, n, path, errorCount);
				}
			}

			else if (strcmp(argv[i], "-w")==0 || strcmp(argv[i], "--window")==0 || strncmp(argv[i], "--window=",9)==0) {
				long long timeInterval;
				if (strcmp(argv[i], "-w")==0 || strcmp(argv[i], "--window")==0) {
					if (i < argc - 1) {
						timeInterval = atoi(argv[i + 1]);
						i++;
						if (timeInterval != 0) {
							MaxRequestsInWindow(log, timeInterval, from, to);
						}
					}
					continue;
				}
				else {
					timeInterval = atoi(Substring(argv[i], 9, strlen(argv[i])));
					if (timeInterval != 0) {
						MaxRequestsInWindow(log, timeInterval, from, to);
					}
				}
			}

			else if (!(strcmp(argv[i], "-f")==0 || strcmp(argv[i], "--from")==0
				|| strncmp(argv[i], "--from=", 7)==0
				|| strcmp(argv[i], "-t")==0 || strcmp(argv[i], "--to")==0
				|| strncmp(argv[i], "--to=", 5)==0
				|| strcmp(argv[i], "-p")==0 || strcmp(argv[i], "--print")==0
				|| strcmp(argv[i], "-o")==0 || strcmp(argv[i], "--output")==0
				|| strncmp(argv[i], "--output=", 9)==0)) {
				PrintError(2);
				return 1;
			}
		}
	}
	return 0;
}


bool IsLog(char* argument) {
	if (argument[0] == '-') {
		if (argument[1] == '-') {
			if (strncmp(argument, "--output=", 9)==0 ||
				strcmp(argument, "--output")==0 ||
				strcmp(argument, "--print")==0 ||
				strncmp(argument, "--stats=", 8)==0 ||
				strcmp(argument, "--stats")==0 ||
				strncmp(argument, "--window=", 9) ==0||
				strcmp(argument, "--window")==0 ||
				strncmp(argument, "--from=", 7)==0 ||
				strcmp(argument, "--from")==0 ||
				strncmp(argument, "--to=", 5)==0 ||
				strcmp(argument, "--to")==0) {
				return false;
			}
		}
		else {
			if (strcmp(argument, "-o")==0 || strcmp(argument, "-p")==0
				|| strcmp(argument, "-s")==0 || strcmp(argument, "-w")==0
				|| strcmp(argument, "-f")==0 || strcmp(argument, "-t")==0) {
				return false;
			}
		}
	}
	return true;
}


int main(int argc, char** argv) {
	if (argc < 2) {
		PrintError(3);
		return 1;
	}
	char* log;
	if (IsLog(argv[1])) {
		log = argv[1];
	}
	else {
		log = argv[argc - 1];
	}
	std::ifstream logFile(log);
	if (!logFile.is_open()) {
		PrintError(1);
		return 1;
	}
	ParseArguments(argc, argv, log);
	return 0;
}
