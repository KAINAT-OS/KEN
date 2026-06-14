// ============================================================================
//  🎌 ken.hpp (v13.0 - Hardened High-Level Systems Scripting Architecture)
//  Zero-Cost Abstractions: Fluid high-level syntax compiling to raw machine code.
// ============================================================================
#ifndef KEN_HPP
#define KEN_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <memory>
#include <optional>
#include <expected>
#include <concepts>
#include <algorithm>
#include <functional>
#include <print>
#include <type_traits>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <array>
#include <tuple>
#include <cctype>
#include <utility>
#include <sys/wait.h>

namespace ken {

    // ============================================================================
    // 1. TYPOGRAPHY, PRIMITIVE SYSTEM ALIASES & CORE CONTAINERS
    // ============================================================================
    using String = std::string;
    using Int    = int64_t;
    using Float  = double;
    using Bool   = bool;

    template <typename T> using List = std::vector<T>;
    template <typename K, typename V> using Dict = std::unordered_map<K, V>;
    template <typename... Ts> using Tuple = std::tuple<Ts...>;

    constexpr std::nullopt_t None = std::nullopt;

    // ============================================================================
    // 2. SUBPROCESS, OS ABSTRACT CHANNELS & RESULT DATA PIPES
    // ============================================================================
    struct CommandResult {
        String output;
        Int exit_code;

        // Implicit type conversion fallback allows treating the output directly as a String
        operator String() const { return output; }
    };

    class OS {
    private:
        OS() = default;

    public:
        OS(const OS&) = delete;
        OS& operator=(const OS&) = delete;

        static OS& get_singleton() {
            static OS instance;
            return instance;
        }

        CommandResult execute(const String& command) {
            std::array<char, 256> buffer;
            String result_output;

            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) return { "Error: Failed to allocate system process pipeline wrapper context.", -1 };

            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result_output += buffer.data();
            }

            int close_status = pclose(pipe);
            Int clear_exit_code = -1;

            if (close_status != -1) {
                if (WIFEXITED(close_status)) {
                    clear_exit_code = WEXITSTATUS(close_status);
                }
            }

            return { result_output, clear_exit_code };
        }

        void delay_msec(Int msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }
        void delay_sec(Float sec) { std::this_thread::sleep_for(std::chrono::duration<double>(sec)); }
        void exit(int code = 0) { std::exit(code); }
    };

    inline OS& os = OS::get_singleton();

    // ============================================================================
    // 3. REFLECTIVE TYPE INSPECTION DETECTORS (SFINAE Template Engine)
    // ============================================================================
    namespace detail {
        template <typename, typename = void> struct has_begin_end : std::false_type {};
        template <typename T> struct has_begin_end<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::true_type {};

        template <typename T> struct is_pair : std::false_type {};
        template <typename T, typename U> struct is_pair<std::pair<T, U>> : std::true_type {};

        template <typename T> struct is_tuple : std::false_type {};
        template <typename... Args> struct is_tuple<std::tuple<Args...>> : std::true_type {};
    }

    // ============================================================================
    // 4. DEEP POLYMORPHIC PYTHONIC VARIADIC PRINT ENGINE
    // ============================================================================
    struct end {
        String val;
        explicit end(const char* s) : val(s) {}
        explicit end(String s) : val(std::move(s)) {}
    };

    namespace detail {
        template <typename T> struct is_end_tag : std::false_type {};
        template <> struct is_end_tag<end> : std::true_type {};

        // Forward declaration to manage structural cross-evaluation recursion loops
        template <typename T> void print_element(const T& val);

        template<typename T, std::size_t... Is>
        void print_tuple_contents(const T& t, std::index_sequence<Is...>) {
            std::print("(");
            (..., (std::print("{}", Is == 0 ? "" : ", "), print_element(std::get<Is>(t))));
            std::print(")");
        }

        template <typename T>
        void print_element(const T& val) {
            if constexpr (std::is_same_v<T, Bool>) {
                std::print("{}", val ? "true" : "false");
            } else if constexpr (std::is_same_v<T, CommandResult>) {
                std::print("{}", val.output);
            } else if constexpr (std::is_convertible_v<T, String> || std::is_convertible_v<T, const char*>) {
                std::print("{}", val);
            } else if constexpr (is_pair<T>::value) {
                print_element(val.first);
                std::print(": ");
                print_element(val.second);
            } else if constexpr (is_tuple<T>::value) {
                print_tuple_contents(val, std::make_index_sequence<std::tuple_size_v<T>>{});
            } else if constexpr (has_begin_end<T>::value) {
                std::print("[");
                bool first = true;
                for (const auto& item : val) {
                    if (!first) std::print(", ");
                    print_element(item);
                    first = false;
                }
                std::print("]");
            } else {
                std::print("{}", val);
            }
        }
    }

    template <typename... Args>
    void print(Args&&... args) {
        if constexpr (sizeof...(args) == 0) {
            std::print("\n");
            return;
        } else {
            auto tuple_args = std::forward_as_tuple(args...);
            constexpr size_t last_idx = sizeof...(args) - 1;
            auto&& last_arg = std::get<last_idx>(tuple_args);

            if constexpr (detail::is_end_tag<std::decay_t<decltype(last_arg)>>::value) {
                size_t current_idx = 0;
                (([&](const auto& val) {
                    if (current_idx < last_idx) {
                        detail::print_element(val);
                        if (current_idx < last_idx - 1) std::print(" ");
                        current_idx++;
                    }
                })(args), ...);
                std::print("{}", last_arg.val);
                std::cout.flush();
            } else {
                size_t current_idx = 0;
                (([&](const auto& val) {
                    detail::print_element(val);
                    if (current_idx < last_idx) std::print(" ");
                    current_idx++;
                })(args), ...);
                std::print("\n");
            }
        }
    }

    inline String input(const String& prompt = "") {
        if (!prompt.empty()) { std::print("{}", prompt); std::cout.flush(); }
        String line;
        std::getline(std::cin, line);
        return line;
    }

    template <typename C>
    inline Int len(const C& container) { return static_cast<Int>(container.size()); }

    // ============================================================================
    // 5. INLINED HIGH-PERFORMANCE LOOP GENERATORS & ITERATORS
    // ============================================================================
    inline List<Int> range(Int stop) {
        List<Int> res;
        if (stop > 0) {
            res.reserve(stop);
            for (Int i = 0; i < stop; ++i) res.push_back(i);
        }
        return res;
    }

    inline List<Int> range(Int start, Int stop, Int step = 1) {
        List<Int> res;
        if (step == 0) return res;
        if (step > 0) {
            if (stop > start) res.reserve((stop - start) / step + 1);
            for (Int i = start; i < stop; i += step) res.push_back(i);
        } else {
            if (start > stop) res.reserve((start - stop) / (-step) + 1);
            for (Int i = start; i > stop; i += step) res.push_back(i);
        }
        return res;
    }

    template <typename F>
    inline void times(Int iterations, F&& lambda) {
        for (Int i = 0; i < iterations; ++i) {
            std::forward<F>(lambda)(i);
        }
    }

    template <typename C, typename B>
    inline void while_loop(C&& condition, B&& body) {
        while (std::forward<C>(condition)()) {
            std::forward<B>(body)();
        }
    }

    // ============================================================================
    // 6. JAVA-STYLE OBJECT EXTENSION STRING UTILITIES
    // ============================================================================
    class StringUtils {
    public:
        static List<String> split(const String& str, char delimiter) {
            List<String> tokens;
            String token;
            std::istringstream tokenStream(str);
            while (std::getline(tokenStream, token, delimiter)) {
                tokens.push_back(token);
            }
            return tokens;
        }

        static String join(const List<String>& elements, const String& delimiter) {
            if (elements.empty()) return "";
            std::ostringstream os_stream;
            for (size_t i = 0; i < elements.size(); ++i) {
                os_stream << elements[i];
                if (i < elements.size() - 1) os_stream << delimiter;
            }
            return os_stream.str();
        }

        static Bool contains(const String& str, const String& substring) {
            return str.find(substring) != String::npos;
        }

        static String replace(String str, const String& from, const String& to) {
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != String::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
            return str;
        }
    };

    // ============================================================================
    // 7. HIGH-PERFORMANCE BARE-METAL FILE I/O PIPES
    // ============================================================================
    inline String read_file(const String& file_path) {
        std::ifstream file(file_path, std::ios::in | std::ios::binary);
        if (!file.is_open()) return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    inline Bool write_file(const String& file_path, const String& content) {
        std::ofstream file(file_path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return false;
        file << content;
        return true;
    }

    // ============================================================================
    // 8. SYNTAX FACTORY GENERATORS (klist, kdict, string parsers)
    // ============================================================================
    template <typename K, typename V>
    inline auto pair(K&& key, V&& value) {
        return std::make_pair(std::forward<K>(key), std::forward<V>(value));
    }

    template <typename T>
    inline List<T> klist(std::initializer_list<T> init_sequence) {
        List<T> local_vector;
        local_vector.reserve(init_sequence.size());
        local_vector.assign(init_sequence.begin(), init_sequence.end());
        return local_vector;
    }

    template <typename... Args>
        requires (sizeof...(Args) > 0)
    inline auto klist(Args&&... args) {
        using DeducedType = std::common_type_t<std::decay_t<Args>...>;
        List<DeducedType> local_vector;
        local_vector.reserve(sizeof...(args));
        (local_vector.push_back(std::forward<Args>(args)), ...);
        return local_vector;
    }

    inline List<Int> klist() { return List<Int>{}; }

    template <typename... Args>
        requires (sizeof...(Args) > 0)
    inline auto kdict(Args&&... args) {
        using FirstPair = std::tuple_element_t<0, std::tuple<std::decay_t<Args>...>>;
        using K = typename FirstPair::first_type;
        using V = typename FirstPair::second_type;

        Dict<K, V> local_map;
        local_map.reserve(sizeof...(args));
        (local_map.insert(std::forward<Args>(args)), ...);
        return local_map;
    }

    template <typename K, typename V>
    inline Dict<K, V> kdict(std::initializer_list<std::pair<const K, V>> init_sequence) {
        Dict<K, V> local_map;
        local_map.reserve(init_sequence.size());
        for (const auto& item : init_sequence) { local_map.insert(item); }
        return local_map;
    }

    inline Dict<String, String> kdict() { return Dict<String, String>{}; }

    inline Dict<String, String> kdict_from_str(String raw_payload) {
        Dict<String, String> generated_map;
        if (!raw_payload.empty() && raw_payload.front() == '{') raw_payload.erase(0, 1);
        if (!raw_payload.empty() && raw_payload.back() == '}') raw_payload.pop_back();

        List<String> entries = StringUtils::split(raw_payload, ',');
        for (const auto& entry : entries) {
            size_t colon_pos = entry.find(':');
            if (colon_pos == String::npos) continue;

            String raw_key   = entry.substr(0, colon_pos);
            String raw_value = entry.substr(colon_pos + 1);

            auto trim_edges = [](String& target) {
                target.erase(target.begin(), std::find_if(target.begin(), target.end(), [](unsigned char ch) { return !std::isspace(ch); }));
                target.erase(std::find_if(target.rbegin(), target.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), target.end());
            };

            trim_edges(raw_key);
            trim_edges(raw_value);

            if (!raw_key.empty()) generated_map[raw_key] = raw_value;
        }
        return generated_map;
    }

    // ============================================================================
    // 9. NATIVE BARE-METAL JSON PARSER (JSON Engine)
    // ============================================================================
    class JSON {
    private:
        static void skip_whitespace(const String& str, size_t& index) {
            while (index < str.size() && (str[index] == ' ' || str[index] == '\t' || str[index] == '\n' || str[index] == '\r')) {
                index++;
            }
        }

        static String parse_string(const String& str, size_t& index) {
            index++;
            size_t start = index;
            while (index < str.size() && str[index] != '"') {
                if (str[index] == '\\') index++;
                index++;
            }
            String res = str.substr(start, index - start);
            index++;
            return res;
        }

    public:
        static String parse_value(const String& str, size_t& index, Dict<String, String>& flat_map, const String& prefix = "") {
            skip_whitespace(str, index);
            if (index >= str.size()) return "";

            if (str[index] == '{') {
                index++;
                while (index < str.size()) {
                    skip_whitespace(str, index);
                    if (str[index] == '}') { index++; break; }

                    String key = parse_string(str, index);
                    skip_whitespace(str, index);
                    if (str[index] == ':') index++;

                    String full_key = prefix.empty() ? key : prefix + "." + key;
                    String val = parse_value(str, index, flat_map, full_key);
                    flat_map[full_key] = val;

                    skip_whitespace(str, index);
                    if (str[index] == ',') index++;
                }
                return "[Object]";
            }
            else if (str[index] == '[') {
                index++;
                String array_accumulator = "[";
                bool first = true;
                while (index < str.size()) {
                    skip_whitespace(str, index);
                    if (str[index] == ']') { index++; break; }
                    if (!first) array_accumulator += ", ";

                    if (str[index] == '"') {
                        array_accumulator += parse_string(str, index);
                    } else {
                        size_t start = index;
                        while (index < str.size() && str[index] != ',' && str[index] != ']') index++;
                        array_accumulator += str.substr(start, index - start);
                    }
                    first = false;
                    skip_whitespace(str, index);
                    if (str[index] == ',') index++;
                }
                array_accumulator += "]";
                return array_accumulator;
            }
            else if (str[index] == '"') {
                return parse_string(str, index);
            } else {
                size_t start = index;
                while (index < str.size() && str[index] != ',' && str[index] != '}' && str[index] != ']') index++;
                String val = str.substr(start, index - start);
                while(!val.empty() && std::isspace(val.back())) val.pop_back();
                return val;
            }
        }

        static Dict<String, String> read_string(const String& json_string) {
            Dict<String, String> target_map;
            size_t tracking_index = 0;
            parse_value(json_string, tracking_index, target_map);
            return target_map;
        }

        static Dict<String, String> read_file(const String& file_path) {
            std::ifstream file(file_path);
            if (!file.is_open()) return {};
            std::stringstream buffer;
            buffer << file.rdbuf();
            return read_string(buffer.str());
        }
    };

    // ============================================================================
    // 10. HIGH-RESOLUTION REAL-TIME PROFILING BENCHMARK TIMERS
    // ============================================================================
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
    public:
        Timer() { reset(); }
        void reset() { start_time = std::chrono::high_resolution_clock::now(); }

        Float elapsed() {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end_time - start_time;
            return duration.count();
        }

        template <typename F>
        static void benchmark(const String& label, F&& block) {
            auto start = std::chrono::high_resolution_clock::now();
            std::forward<F>(block)();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            ken::print("[BENCHMARK]", label, "finalized processing window in:", duration.count(), "ms");
        }
    };

    // ============================================================================
    // 11. BASH SH-STREAM OPERATOR FLOW OVERLOADS
    // ============================================================================
    struct BashPipe {};
    inline BashPipe bash;
    inline CommandResult operator|(const String& cmd, [[maybe_unused]] BashPipe b) { return os.execute(cmd); }
    inline CommandResult operator|(const char* cmd, [[maybe_unused]] BashPipe b) { return os.execute(String(cmd)); }

} // namespace ken

// ============================================================================
// 12. CLEAN RE-MAPPING OF INFERENCE PATTERNS
// ============================================================================
using namespace ken;

// Fluid Type Inference Handles (Maps natively to C++23 structural deduction auto)
#define var  auto
#define func auto

#endif // KEN_HPP
