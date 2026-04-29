#include "ArgParser.h"

namespace ArgumentParser {

std::vector<std::string> positional;

enum class ArgType { Int, Flag, String, Help };

Argument::Argument(std::string name, char short_name, std::string description,
                   ArgType type)
    : name_(name),
      short_name_(short_name),
      description_(description),
      type_(type) {
        if(type==ArgType::Int){
            int* storage=new int;
            value_=storage;
        }
        else if(type==ArgType::Flag){
            bool* storage = new bool;
            value_=storage;
        }
        else if(type==ArgType::String){
            std::string* storage=new std::string;
            value_=storage;
        }
      }

Argument::Argument() : name_(""), short_name_('\0'), description_(""), type_(ArgType::String) {}

Argument& Argument::MultiValue() {
    multi_value_ = true;
    if (type_ == ArgType::Int) {
        std::vector<int>* tmp = new std::vector<int>;
        if (count_ != 0) {
            (*tmp)[0] = *static_cast<int*>(value_);
        }
        value_ = tmp;
    } else if (type_ == ArgType::Flag) {
        std::vector<bool>* tmp = new std::vector<bool>;
        if (count_ != 0) {
            (*tmp)[0] = *static_cast<bool*>(value_);
        }
        value_ = tmp;
    } else {
        std::vector<std::string>* tmp = new std::vector<std::string>;
        if (count_ != 0) {
            (*tmp)[0] = *static_cast<std::string*>(value_);
        }
        value_ = tmp;
    }
    return *this;
}

Argument& Argument::MultiValue(size_t min_count) {
    MultiValue();
    min_count_ = min_count;
    return *this;
}

Argument& Argument::Default(const int& value) {
    int* tmp = new int;
    *tmp = value;
    if (multi_value_) {
        (*static_cast<std::vector<int>*>(value_))[0] = *tmp;
    } else {
        value_ = tmp;
    }
    count_=1;
    return *this;
}

Argument& Argument::Default(const bool& value) {
    bool* tmp = new bool;
    *tmp = value;
    if (multi_value_) {
        (*static_cast<std::vector<bool>*>(value_))[0] = *tmp;
    } else {
        value_ = tmp;
    }
    count_=1;
    return *this;
}

Argument& Argument::Default(const std::string& value) {
    std::string* tmp = new std::string(value);
    if (multi_value_) {
        *static_cast<std::string*>(value_) = *tmp;
    }
    else {
        value_ = tmp;
    }
    count_=1;
    return *this;
}

Argument& Argument::Default(const char* value) {
    return Default(std::string(value));
}

Argument& Argument::Default(const std::vector<int>& values) {
    if (multi_value_) {
        std::vector<int>* tmp = new std::vector<int>;
        *tmp = values;
        if (multi_value_) {
            *static_cast<std::vector<int>*>(value_) = *tmp;
        } else {
            value_ = tmp;
        }
        count_=values.size();
    }
    return *this;
}

Argument& Argument::Default(const std::vector<bool>& values) {
    if (multi_value_) {
        std::vector<bool>* tmp = new std::vector<bool>;
        *tmp = values;
        if (multi_value_) {
            *static_cast<std::vector<bool>*>(value_) = *tmp;
        } else {
            value_ = tmp;
        }
        count_=values.size();
    }
    return *this;
}

Argument& Argument::Default(const std::vector<std::string>& values) {
    if (multi_value_) {
        std::vector<std::string>* tmp = new std::vector<std::string>;
        *tmp = values;
        if (multi_value_) {
            *static_cast<std::vector<std::string>*>(value_) = *tmp;
        } else {
            value_ = tmp;
        }
        count_=values.size();
    }
    return *this;
}

Argument& Argument::StoreValues(int& value) {
    if (count_ != 0) {
        value = *static_cast<int*>(value_);
    }
    delete static_cast<int*>(value_);
    value_ = &value;
    return *this;
}

Argument& Argument::StoreValues(bool& value) {
   if (count_ != 0) {
        value = *static_cast<bool*>(value_);
    }
    delete static_cast<bool*>(value_);
    value_ = &value;
    return *this;
}

Argument& Argument::StoreValues(std::string& value) {
    if (count_ != 0) {
        value = *static_cast<std::string*>(value_);
    }
    delete static_cast<std::string*>(value_);
    value_ = &value;
    return *this;
}

Argument& Argument::StoreValues(std::vector<int>& values) {
    if (multi_value_) {
        if (count_ != 0) {
            values = *static_cast<std::vector<int>*>(value_);
        }
        delete static_cast<std::vector<int>*>(value_);
        value_ = &values;
    }
    return *this;
}

Argument& Argument::StoreValues(std::vector<bool>& values) {
    if (multi_value_) {
        if (count_ != 0) {
            values = *static_cast<std::vector<bool>*>(value_);
        }
        delete static_cast<std::vector<bool>*>(value_);
        value_ = &values;
    }
    return *this;
}

Argument& Argument::StoreValues(std::vector<std::string>& values) {
    if (multi_value_) {
        if (count_ != 0) {
            values = *static_cast<std::vector<std::string>*>(value_);
        }
        delete static_cast<std::vector<std::string>*>(value_);
        value_ = &values;
    }
    return *this;
}

Argument& Argument::Positional() {
    positional_ = true;
    positional.push_back(name_);
    return *this;
}

void* Argument::GetValue() { return value_; }

ArgType Argument::GetType() { return type_; }

bool Argument::IsMultiValue() { return multi_value_; };

bool Argument::IsPositional(){ return positional_;};

size_t& Argument::GetCount(){
    return count_;
}

size_t& Argument::GetMinCount(){
    return min_count_;
}


ArgParser::ArgParser(std::string name) : name_(name) {}

Argument& ArgParser::AddStringArgument(const std::string& name) {
    return AddStringArgument('\0', name, "");
}

Argument& ArgParser::AddStringArgument(const std::string& name,
                            const std::string& description) {
    return AddStringArgument('\0', name, description);
}

Argument& ArgParser::AddStringArgument(const char& short_name, const std::string& name) {
    return AddStringArgument(short_name, name, "");
}

Argument& ArgParser::AddStringArgument(const char& short_name, const std::string& name,
                            const std::string& description) {
    arguments_[name] = Argument(name, short_name, description, ArgType::String);
    if(short_name!='\0'){
        short_args_[short_name] = name;
    }
    return arguments_[name];
}

Argument& ArgParser::AddIntArgument(const std::string& name) {
    return AddIntArgument('\0', name, "");
}

Argument& ArgParser::AddIntArgument(const std::string& name,
                         const std::string& description) {
    return AddIntArgument('\0', name, description);
}

Argument& ArgParser::AddIntArgument(const char& short_name, const std::string& name) {
    return AddIntArgument(short_name, name, "");
}

Argument& ArgParser::AddIntArgument(const char& short_name, const std::string& name,
                         const std::string& description) {
    arguments_[name] = Argument(name, short_name, description, ArgType::Int);
    if(short_name!='\0'){
        short_args_[short_name] = name;
    }
    return arguments_[name];
}

Argument& ArgParser::AddFlag(const std::string& name) {
    return AddFlag('\0', name, "");
}

Argument& ArgParser::AddFlag(const std::string& name, const std::string& description) {
    return AddFlag('\0', name, description);
}

Argument& ArgParser::AddFlag(const char& short_name, const std::string& name) {
    return AddFlag(short_name, name, "");
}

Argument& ArgParser::AddFlag(const char& short_name, const std::string& name,
                  const std::string& description) {
    arguments_[name] = Argument(name, short_name, description, ArgType::Flag);
    if(short_name!='\0'){
        short_args_[short_name] = name;
    }
    return arguments_[name];
}

ArgParser& ArgParser::AddHelp(const char& short_name, const std::string& name, const std::string& description){
    arguments_[name] = Argument(name, short_name, description, ArgType::Help);
    short_args_[short_name] = name;
    return *this;
}

bool ArgParser::Parse(const std::vector<std::string>& args) {
    help=false;
    int pos_idx = 0;
    for (int i = 1; i < args.size(); i++) {
        std::string argument;
        if (args[i][0] == '-') {
            if (args[i][1] == '-') {
                size_t idx = args[i].find('=');
                std::string arg_value;
                if (idx == std::string::npos) {
                    argument = args[i].substr(2);
                    if(arguments_[argument].GetType()!=ArgType::Help){
                        i++;
                        if(i<args.size()){
                            arg_value = args[i];
                        }
                    }
                }
                else {
                    argument = args[i].substr(2, idx-2);
                    if(idx+1<args[i].length()){
                        arg_value = args[i].substr(idx + 1);
                    }
                }
                if (arguments_.find(argument) != arguments_.end()) {
                    if (arguments_[argument].GetType() == ArgType::Int) {
                        if (IsDigit(arg_value)) {
                            if(arguments_[argument].IsMultiValue()){
                                (*static_cast<std::vector<int>*>(
                                arguments_[argument].GetValue())).push_back(std::stoi(arg_value));
                            }
                            else{
                            *static_cast<int*>(
                                arguments_[argument].GetValue()) =
                                std::stoi(arg_value);
                            }
                            arguments_[argument].GetCount()++;
                        }
                    } else if (arguments_[argument].GetType() ==
                               ArgType::Flag) {
                        if(arguments_[argument].IsMultiValue()){
                                (*static_cast<std::vector<bool>*>(
                                arguments_[argument].GetValue())).push_back(true);
                        }
                        else{
                            *static_cast<bool*>(arguments_[argument].GetValue()) =
                                true;
                        }
                        arguments_[argument].GetCount()++;
                    } else if (arguments_[argument].GetType() ==
                               ArgType::String) {
                        if(arguments_[argument].IsMultiValue()){
                                (*static_cast<std::vector<std::string>*>(
                                arguments_[argument].GetValue())).push_back(arg_value);
                        }
                        else{
                            *static_cast<std::string*>(
                                arguments_[argument].GetValue()) = arg_value;
                        }
                        arguments_[argument].GetCount()++;
                    }
                    else{
                        arguments_[argument].GetCount()++;
                        help=true;
                    }
                }
            }
            
            else {
                size_t idx = args[i].find('=');
                if (idx == std::string::npos) {
                    argument = args[i].substr(1);
                    if(arguments_[short_args_[argument[0]]].GetType() == ArgType::Help){
                            help=true;
                            arguments_[short_args_[argument[0]]].GetCount()++;
                    }
                    else{
                    if (arguments_[short_args_[argument[0]]].GetType() ==
                        ArgType::Flag) {
                        for (int j = 0; j < argument.length(); j++) {
                            if (arguments_.find(short_args_[argument[j]]) ==
                                arguments_.end()) {
                                return false;
                            }
                            if(arguments_[short_args_[argument[j]]].IsMultiValue()){
                                (*static_cast<std::vector<bool>*>(
                                arguments_[short_args_[argument[j]]].GetValue())).push_back(true);
                            }
                            else{
                            *static_cast<bool*>(
                                arguments_[short_args_[argument[j]]]
                                    .GetValue()) = true;
                            }
                            arguments_[short_args_[argument[j]]].GetCount()++;
                        }
                    } else {
                        if (argument.length() > 1) {
                            return false;
                        }
                        i++;
                        std::string arg_value = args[i];
                        if (arguments_[short_args_[argument[0]]].GetType() ==
                            ArgType::Int) {
                            if (IsDigit(arg_value)) {
                                if(arguments_[short_args_[argument[0]]].IsMultiValue()){
                                    (*static_cast<std::vector<int>*>(
                                    arguments_[short_args_[argument[0]]].GetValue())).push_back(std::stoi(arg_value));
                                }
                                else{
                                *static_cast<int*>(
                                    arguments_[short_args_[argument[0]]]
                                        .GetValue()) = std::stoi(arg_value);
                                }
                                arguments_[short_args_[argument[0]]].GetCount()++;

                            }
                        } else if (arguments_[short_args_[argument[0]]]
                                       .GetType() == ArgType::String) {
                            if(arguments_[short_args_[argument[0]]].IsMultiValue()){
                                    (*static_cast<std::vector<std::string>*>(
                                    arguments_[short_args_[argument[0]]].GetValue())).push_back(arg_value);
                            } 
                            else{           
                                *static_cast<std::string*>(
                                    arguments_[short_args_[argument[0]]]
                                        .GetValue()) = arg_value;
                            }
                            arguments_[short_args_[argument[0]]].GetCount()++;
                        }
                    }
                    }

                }
                else {
                    argument = args[i].substr(1, idx - 1);
                    if (argument.length() > 1) {
                        return false;
                    }
                    std::string arg_value = args[i].substr(idx + 1);
                    if (arguments_[short_args_[argument[0]]].GetType() ==
                        ArgType::Int) {
                        if (IsDigit(arg_value)) {
                            if(arguments_[short_args_[argument[0]]].IsMultiValue()){
                                    (*static_cast<std::vector<int>*>(
                                    arguments_[short_args_[argument[0]]].GetValue())).push_back(std::stoi(arg_value));
                            }
                            else{
                            *static_cast<int*>(
                                arguments_[short_args_[argument[0]]]
                                    .GetValue()) = std::stoi(arg_value);
                            }
                            arguments_[short_args_[argument[0]]].GetCount()++;
                        }
                    } else if (arguments_[short_args_[argument[0]]].GetType() ==
                               ArgType::String) {
                        if(arguments_[short_args_[argument[0]]].IsMultiValue()){
                            (*static_cast<std::vector<std::string>*>(
                            arguments_[short_args_[argument[0]]].GetValue())).push_back(arg_value);
                        }
                        else{
                            *static_cast<std::string*>(
                                arguments_[short_args_[argument[0]]].GetValue()) =
                                arg_value;
                        }
                        arguments_[short_args_[argument[0]]].GetCount()++;
                    }
                }
            }
        }
        else {
            std::string arg_value=args[i];
            if(IsDigit(arg_value)){
                while(pos_idx<positional.size() && arguments_[positional[pos_idx]].GetType()!=ArgType::Int){
                    pos_idx++;
                }
                if(pos_idx==positional.size()){
                    return false;
                }
                if(arguments_[positional[pos_idx]].IsMultiValue()){
                    (*static_cast<std::vector<int>*>(arguments_[positional[pos_idx]]
                                    .GetValue())).push_back(std::stoi(arg_value));
                    arguments_[positional[pos_idx]]
                                    .GetCount()++;
                }
                else{
                    *static_cast<int*>(arguments_[positional[pos_idx]]
                                    .GetValue()) = std::stoi(arg_value);
                    arguments_[positional[pos_idx]]
                                    .GetCount()++;
                    pos_idx++;
                }
            }
            else{
                while(pos_idx<positional.size() && arguments_[positional[pos_idx]].GetType()!=ArgType::String){
                    pos_idx++;
                }
                if(pos_idx==positional.size()){
                    return false;
                }
                if(arguments_[positional[pos_idx]].IsMultiValue()){
                    (*static_cast<std::vector<std::string>*>(arguments_[positional[pos_idx]]
                                    .GetValue())).push_back(arg_value);
                    arguments_[positional[pos_idx]]
                                    .GetCount()++;
                }
                else{
                    *static_cast<std::string*>(arguments_[positional[pos_idx]]
                                    .GetValue()) = arg_value;
                    arguments_[positional[pos_idx]]
                                    .GetCount()++;
                    pos_idx++;
                }
            }
        }
    }
    for (auto& pair : arguments_) {
        if(!help && pair.second.GetCount()<pair.second.GetMinCount() && pair.second.GetType()!=ArgType::Flag && pair.second.GetType()!=ArgType::Help){
            return false;
        }
    }
    return true;
}

std::string ArgParser::GetStringValue(std::string name) {
    return *static_cast<std::string*>(arguments_[name].GetValue());
}

std::string ArgParser::GetStringValue(std::string name, int index){
    return (*static_cast<std::vector<std::string>*>(arguments_[name].GetValue()))[index];
}

int ArgParser::GetIntValue(std::string name) {
    return *static_cast<int*>(arguments_[name].GetValue());
}

int ArgParser::GetIntValue(std::string name, int index){
    return (*static_cast<std::vector<int>*>(arguments_[name].GetValue()))[index];
}

bool ArgParser::GetFlag(std::string name) {
    return *static_cast<bool*>(arguments_[name].GetValue());
}

bool ArgParser::GetFlag(std::string name, int index){
    return (*static_cast<std::vector<bool>*>(arguments_[name].GetValue()))[index];
}

bool ArgParser::Help(){
    return help;
}

std::string ArgParser::HelpDescription(){
    std::string help_desc;
    help_desc="My Parser\n Some description\n";
    std::string i;
    for (auto& pair : arguments_) {
        i=pair.first;
        help_desc+=i;
        help_desc+="\n";
    }
    return help_desc;
}

bool IsDigit(std::string str) {
    for (int i = 0; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

}  // namespace ArgumentParser
