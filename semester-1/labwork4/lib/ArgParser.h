#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ArgumentParser {

enum class ArgType;

class Argument {
   public:
    Argument(std::string name, char short_name, std::string description, ArgType type);

    Argument();

    Argument& MultiValue();

    Argument& MultiValue(size_t min_count);

    Argument& Default(const int& value);

    Argument& Default(const bool& value);

    Argument& Default(const std::string& value);

    Argument& Default(const std::vector<int>& values);

    Argument& Default(const std::vector<bool>& values);

    Argument& Default(const std::vector<std::string>& values);

    Argument& Default(const char* value);

    Argument& StoreValues(int& value);

    Argument& StoreValues(bool& value);

    Argument& StoreValues(std::string& value);

    Argument& StoreValues(std::vector<int>& values);

    Argument& StoreValues(std::vector<bool>& values);

    Argument& StoreValues(std::vector<std::string>& values);

    Argument& Positional();

    void* GetValue();

    ArgType GetType();

    bool IsMultiValue();

    bool IsPositional();

    size_t& GetCount();

    size_t& GetMinCount();

   private:
    std::string name_;
    char short_name_;
    std::string description_;
    ArgType type_;
    void* value_=nullptr;
    bool multi_value_=false;
    bool positional_=false;
    bool store_values_=false;
    size_t min_count_=1;
    size_t count_=0;
};

class ArgParser {
   public:
    ArgParser(std::string name);

    Argument& AddStringArgument(const std::string& name);

    Argument& AddStringArgument(const std::string& name, const std::string& description);

    Argument& AddStringArgument(const char& short_name, const std::string& name);

    Argument& AddStringArgument(const char& short_name, const std::string& name, const std::string& description);

    Argument& AddIntArgument(const std::string& name);
    
    Argument& AddIntArgument(const std::string& name, const std::string& description);

    Argument& AddIntArgument(const char& short_name, const std::string& name);

    Argument& AddIntArgument(const char& short_name, const std::string& name, const std::string& description);

    Argument& AddFlag(const std::string& name);

    Argument& AddFlag(const std::string& name, const std::string& description);

    Argument& AddFlag(const char& short_name, const std::string& name);

    Argument& AddFlag(const char& short_name, const std::string& name, const std::string& description);

    ArgParser& AddHelp(const char& short_name, const std::string& name, const std::string& description);

    bool Parse(const std::vector<std::string>& args);

    std::string GetStringValue(std::string name);

    std::string GetStringValue(std::string name, int index);

    int GetIntValue(std::string name);

    int GetIntValue(std::string name, int index);

    bool GetFlag(std::string name);

    bool GetFlag(std::string name, int index);

    bool Help();
    
    std::string HelpDescription();

   private:
    std::string name_;
    std::unordered_map<std::string, Argument> arguments_;
    std::unordered_map<char, std::string> short_args_;
    bool help=false;
};

bool IsDigit(std::string str);

}