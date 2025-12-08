#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <sstream>
#include <fstream>
#include <cctype>
#include <stdexcept>

/*
   Minimal JSON parser for Aerial
   -----------------------------------------
   Supports:
     - objects { "k": v }
     - strings "text"
     - numbers (int)
     - booleans (true/false)
     - contains()
     - get<T>()

   This is NOT a full JSON implementation — it's only
   what we need for reading a simple config.json.
*/

namespace mini_json {

    struct json {
        using object = std::unordered_map<std::string, json>;
        using value = std::variant<std::string, int, bool, object>;

        value data;

        json() = default;
        json(value v) : data(std::move(v)) {}
        json(const char* s) : data(std::string(s)) {}
        json(bool b) : data(b) {}
        json(int i) : data(i) {}

        // -------------------------------------
        // Parse from file
        // -------------------------------------
        static json parseFile(const std::string& path) {
            std::ifstream f(path);
            if (!f.is_open()) {
                throw std::runtime_error("Could not open JSON file: " + path);
            }
            std::stringstream ss;
            ss << f.rdbuf();
            return parse(ss.str());
        }

        // -------------------------------------
        // Parse from string
        // -------------------------------------
        static json parse(const std::string& text) {
            size_t i = 0;
            return parseValue(text, i);
        }

        // -------------------------------------
        // Object helpers
        // -------------------------------------
        bool contains(const std::string& key) const {
            if (std::holds_alternative<object>(data)) {
                const auto& obj = std::get<object>(data);
                return obj.find(key) != obj.end();
            }
            return false;
        }

        const json& operator[](const std::string& key) const {
            const auto& obj = std::get<object>(data);
            return obj.at(key);
        }

        template<typename T>
        T get() const {
            if constexpr (std::is_same_v<T, std::string>) {
                return std::get<std::string>(data);
            }
            if constexpr (std::is_same_v<T, int>) {
                return std::get<int>(data);
            }
            if constexpr (std::is_same_v<T, bool>) {
                return std::get<bool>(data);
            }
            throw std::runtime_error("Unsupported json.get<T>() type");
        }

    private:
        // -------------------------------------
        // Parsing internals
        // -------------------------------------
        static void skipWS(const std::string& s, size_t& i) {
            while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
        }

        static json parseValue(const std::string& s, size_t& i) {
            skipWS(s, i);
            if (i >= s.size())
                throw std::runtime_error("Unexpected end of JSON");

            char c = s[i];
            if (c == '{') return parseObject(s, i);
            if (c == '"') return parseString(s, i);
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') return parseNumber(s, i);
            if (s.compare(i, 4, "true") == 0)  { i += 4; return json(true); }
            if (s.compare(i, 5, "false") == 0) { i += 5; return json(false); }

            throw std::runtime_error("Invalid JSON at index " + std::to_string(i));
        }

        static json parseString(const std::string& s, size_t& i) {
            if (s[i] != '"')
                throw std::runtime_error("Expected '\"' at start of string");
            i++; // skip opening "
            std::string out;
            while (i < s.size() && s[i] != '"') {
                // minimal escape handling: treat \" as "
                if (s[i] == '\\' && i + 1 < s.size() && s[i+1] == '"') {
                    out.push_back('"');
                    i += 2;
                } else {
                    out.push_back(s[i++]);
                }
            }
            if (i >= s.size() || s[i] != '"')
                throw std::runtime_error("Unterminated string in JSON");
            i++; // skip closing "
            return json(out);
        }

        static json parseNumber(const std::string& s, size_t& i) {
            int sign = 1;
            if (s[i] == '-') { sign = -1; i++; }
            int value = 0;
            bool hasDigit = false;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
                hasDigit = true;
                value = value * 10 + (s[i] - '0');
                i++;
            }
            if (!hasDigit)
                throw std::runtime_error("Invalid number in JSON");
            return json(value * sign);
        }

        static json parseObject(const std::string& s, size_t& i) {
            if (s[i] != '{')
                throw std::runtime_error("Expected '{' at start of object");
            i++; // skip '{'
            skipWS(s, i);

            object obj;

            while (i < s.size() && s[i] != '}') {
                // key
                auto keyJson = parseString(s, i);
                std::string key = keyJson.get<std::string>();

                skipWS(s, i);
                if (i >= s.size() || s[i] != ':')
                    throw std::runtime_error("Expected ':' after key");
                i++; // skip ':'

                // value
                auto val = parseValue(s, i);
                obj[key] = val;

                skipWS(s, i);
                if (i < s.size() && s[i] == ',') {
                    i++; // skip ','
                    skipWS(s, i);
                }
            }

            if (i >= s.size() || s[i] != '}')
                throw std::runtime_error("Expected '}' at end of object");
            i++; // skip '}'

            return json(obj);
        }
    };

} // namespace mini_json
