// Argument parser. Not the fastest, but its very easy to use.
// made by bain on 10.01.2021
#include <string>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>

#ifndef VISTE_ARGS_H
#define VISTE_ARGS_H




namespace Args {
    namespace Exceptions {
        class ArgumentParsingError: public std::exception
        {
            std::string msg;
        public:
            explicit ArgumentParsingError(std::string msg) {
                this->msg = std::move(msg);
            }

            [[nodiscard]] const char *what() const noexcept override
            {
                return msg.data();
            }
        };
    }

    struct ArgOpt {
        int position = -1;
        std::string long_name;
        char short_name;
    };

    class Parser {
        int argc;
        char** argv;

    private:
        std::vector<int> not_positional_args;

        int get_string(const ArgOpt& options, const bool& is_bool) {
            int positional_arg = 0;
            for (int i = 0; i < argc; i++) {
                char* argument = argv[i];
                int size_of_argument = strlen(argv[i]);

                // check if keyword argument
                int t = (argument[0] == '-' && size_of_argument > 1);
                t += (argument[1] == '-');
                switch (t) {
                    case 0:
                        // The current string is a positional argument.
                        // Check if argument is not positional
                        if (std::find(not_positional_args.begin(), not_positional_args.end(), i) != not_positional_args.end())
                            continue;
                        if (positional_arg == options.position) return i;
                        positional_arg++;
                        break;
                    case 1:
                        // The current string is a keyword argument with a short name.
                        if (!is_bool && options.short_name == argument[size_of_argument - 1]) {
                            if (argc > i+1) {
                                not_positional_args.push_back(++i);
                                return i;
                            }
                            else throw Exceptions::ArgumentParsingError((std::string)"Argument " + argument + " needs a value. Got none.");
                        } else {
                            for (int j = 0; j < size_of_argument; j++)
                                if (argument[j] == options.short_name) return i;
                        }
                        break;
                    case 2:
                        // The current string is a keyword argument with a long name.
                        if ("--"+options.long_name == argument) {
                            if (!is_bool) {
                                if (argc > i+1) {
                                    not_positional_args.push_back(++i);
                                    return i;
                                }
                                else
                                    throw Exceptions::ArgumentParsingError(
                                            (std::string) "Argument " + argument + " needs a value. Got none.");
                            }
                            else return i;
                        }
                        break;
                    default:
                        break;
                }
            }
            return -1;
        }

    public:
        explicit Parser(int argc, char **argv) {
            this->argc = argc;
            this->argv = argv;
        }

        template<typename ValueType>
        ValueType get_arg(const ArgOpt& options) {
            ValueType output;
            bool val_type_is_bool = std::is_same_v<ValueType, bool>;
            int position = get_string(options, val_type_is_bool);
            if (position == -1) {
                std::string s;
                if (options.long_name.empty()) s+=options.short_name;
                else s+= options.long_name;
                throw Exceptions::ArgumentParsingError(s+" is a required argument.");
            } else {
                if (val_type_is_bool) return true;
                return convert_arg<ValueType>(argv[position]);
            }
        }

        template<typename ValueType>
        ValueType get_arg(const ArgOpt& options, const ValueType& default_) {
            ValueType v;
            try {
                v = get_arg<ValueType>(options);
            }
            catch (Exceptions::ArgumentParsingError& e) {
                return default_;
            }
            return v;
        }

        template<typename ValueType>
        ValueType convert_arg(char *arg) {
            if (std::is_floating_point_v<ValueType>) {
                return (ValueType)(convert_arg<double>(arg));
            } else if (std::is_arithmetic_v<ValueType>) {
                return (ValueType)(convert_arg<long>(arg));
            }
            throw Exceptions::ArgumentParsingError((std::string)"Cannot convert \""+arg+"\"");
        }
    };
}

// template specializations must be in namespace scope

template<>
std::string Args::Parser::get_arg<std::string>(const ArgOpt &options) {
    int position = get_string(options, false);
    if (position == -1) {
        std::string s;
        if (options.long_name.empty()) s+=options.short_name;
        else s+= options.long_name;
        throw Exceptions::ArgumentParsingError(s+" is a required argument.");
    } else {
        return std::string(argv[position]);
    }
}

// converters, yaaay
template<>
long Args::Parser::convert_arg<long>(char *arg) {
    return strtol(arg, nullptr, 10);
}

template<>
double Args::Parser::convert_arg<double>(char *arg) {
    return strtod(arg, nullptr);
}

#endif
