// ============================================================================
//  🎌 ken.hpp (v14.2 – production)
//  C++ as easy as Python, fast as raw metal.
//  Zero-cost abstractions, header-only.
// ============================================================================
#ifndef KEN_HPP
#define KEN_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <chrono>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace ken {

// ============================================================================
// 1. CORE TYPE ALIASES
// ============================================================================
using String = std::string;
using Int    = int64_t;
using Float  = double;
using Bool   = bool;

template <typename T>
using List = std::vector<T>;
template <typename K, typename V>
using Dict = std::unordered_map<K, V>;
template <typename T>
using Set = std::unordered_set<T>;
template <typename... Ts>
using Tuple = std::tuple<Ts...>;
template <typename T>
using Span = std::span<T>;

constexpr std::nullopt_t None = std::nullopt;

// ============================================================================
// 2. LAZY RANGE GENERATOR (zero‑allocation)
// ============================================================================
template <typename T = Int>
class Range {
    T start_, stop_, step_;
public:
    class iterator {
        T current_, step_;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        iterator(T current, T step) : current_(current), step_(step) {}
        T operator*() const { return current_; }
        iterator& operator++() { current_ += step_; return *this; }
        iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
        bool operator==(const iterator& other) const {
            if (step_ > 0) return current_ >= other.current_;
            else            return current_ <= other.current_;
        }
        bool operator!=(const iterator& other) const { return !(*this == other); }
    };

    Range(T start, T stop, T step) : start_(start), stop_(stop), step_(step) {
        if (step == 0) throw std::invalid_argument("range() step must not be zero");
    }

    iterator begin() const { return iterator(start_, step_); }
    iterator end()   const { return iterator(stop_,  step_); }
};

inline Range<Int> range(Int stop) { return Range<Int>(0, stop, 1); }
inline Range<Int> range(Int start, Int stop, Int step = 1) { return Range<Int>(start, stop, step); }

// ============================================================================
// 3. FUNCTIONAL VIEWS (lazy, zero‑cost)
// ============================================================================
template <typename R> auto enumerate(R&& range) { return std::views::enumerate(std::forward<R>(range)); }
template <typename... Rs> auto zip(Rs&&... ranges) { return std::views::zip(std::forward<Rs>(ranges)...); }
template <typename F, typename R> auto map(F&& func, R&& range) {
    return std::forward<R>(range) | std::views::transform(std::forward<F>(func));
}
template <typename P, typename R> auto filter(P&& pred, R&& range) {
    return std::forward<R>(range) | std::views::filter(std::forward<P>(pred));
}
template <typename R, typename T, typename BinaryOp>
auto reduce(R&& range, T init, BinaryOp&& op) {
    return std::accumulate(std::ranges::begin(range), std::ranges::end(range), init, std::forward<BinaryOp>(op));
}
template <typename R, typename BinaryOp>
auto reduce(R&& range, BinaryOp&& op) {
    auto it = std::ranges::begin(range);
    auto end = std::ranges::end(range);
    if (it == end) throw std::invalid_argument("reduce() on empty range with no initial value");
    auto init = *it++;
    return std::accumulate(it, end, init, std::forward<BinaryOp>(op));
}
template <typename R, typename P> Bool any_of(R&& range, P&& pred) {
    return std::ranges::any_of(std::forward<R>(range), std::forward<P>(pred));
}
template <typename R, typename P> Bool all_of(R&& range, P&& pred) {
    return std::ranges::all_of(std::forward<R>(range), std::forward<P>(pred));
}

// ============================================================================
// 4. SLICING & SPAN
// ============================================================================
template <typename T>
List<T> slice(const List<T>& v, Int start, Int stop, Int step = 1) {
    List<T> result;
    if (step == 0) throw std::invalid_argument("slice step must not be zero");
    auto sz = static_cast<Int>(v.size());
    if (start < 0) start += sz;
    if (stop < 0) stop += sz;
    start = std::clamp<Int>(start, 0, sz);
    stop  = std::clamp<Int>(stop, 0, sz);

    if (step > 0) {
        result.reserve((stop - start + step - 1) / step);
        for (auto i = start; i < stop; i += step) result.push_back(v[i]);
    } else {
        result.reserve((start - stop + (-step) - 1) / (-step));
        for (auto i = start; i > stop; i += step) result.push_back(v[i]);
    }
    return result;
}
template <typename T>
Span<T> span(List<T>& v, Int start, Int count) {
    if (start < 0) start += v.size();
    start = std::max<Int>(0, start);
    count = std::min<Int>(count, static_cast<Int>(v.size()) - start);
    return Span<T>(v.data() + start, static_cast<size_t>(count));
}

// ============================================================================
// 5. STRING UTILITIES
// ============================================================================
class StringUtils {
public:
    static List<String> split(const String& str, char delimiter) {
        List<String> tokens;
        std::istringstream tokenStream(str);
        String token;
        while (std::getline(tokenStream, token, delimiter))
            tokens.push_back(token);
        return tokens;
    }
    static String join(const List<String>& elements, const String& delimiter) {
        if (elements.empty()) return "";
        std::ostringstream os;
        os << elements[0];
        for (size_t i = 1; i < elements.size(); ++i)
            os << delimiter << elements[i];
        return os.str();
    }
    static Bool contains(const String& str, const String& substr) { return str.find(substr) != String::npos; }
    static String replace(String str, const String& from, const String& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != String::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
        return str;
    }
    static Bool starts_with(const String& str, const String& prefix) { return str.starts_with(prefix); }
    static Bool ends_with(const String& str, const String& suffix) { return str.ends_with(suffix); }
    static String lower(const String& str) {
        String res = str;
        std::transform(res.begin(), res.end(), res.begin(), ::tolower);
        return res;
    }
    static String upper(const String& str) {
        String res = str;
        std::transform(res.begin(), res.end(), res.begin(), ::toupper);
        return res;
    }
    static String strip(const String& str) {
        auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
        auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (start < end) ? String(start, end) : "";
    }
    static String lstrip(const String& str) {
        auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
        return String(start, str.end());
    }
    static String rstrip(const String& str) {
        auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return String(str.begin(), end);
    }
};

// ============================================================================
// 6. OS SINGLETON (shell, filesystem, environment)
// ============================================================================
struct CommandResult {
    String output;
    Int exit_code;
    operator String() const { return output; }
};

class OS {
    OS() = default;
public:
    OS(const OS&) = delete;
    OS& operator=(const OS&) = delete;
    static OS& get() { static OS instance; return instance; }

    CommandResult execute(const String& command) {
        std::array<char, 256> buffer;
        String result;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) return {"Failed to execute", -1};
        while (fgets(buffer.data(), buffer.size(), pipe))
            result += buffer.data();
        int status = pclose(pipe);
        Int code = -1;
        if (status != -1 && WIFEXITED(status))
            code = WEXITSTATUS(status);
        return {result, code};
    }

    void delay_msec(Int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
    void delay_sec(Float s) { std::this_thread::sleep_for(std::chrono::duration<double>(s)); }
    void exit(int code = 0) { std::exit(code); }

    String getenv(const String& name) {
        const char* val = std::getenv(name.c_str());
        return val ? String(val) : "";
    }
    Bool path_exists(const String& path) { return std::filesystem::exists(path); }
    List<String> listdir(const String& path) {
        List<String> entries;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            entries.push_back(entry.path().filename().string());
        return entries;
    }
    void chdir(const String& path) { std::filesystem::current_path(path); }
    void mkdir(const String& path) { std::filesystem::create_directory(path); }
};

inline OS& os = OS::get();

// ============================================================================
// 7. JSON TYPE + PARSER
// ============================================================================
namespace detail {
    struct JSONValue;
    using JSONList = List<JSONValue>;
    using JSONDict = Dict<String, JSONValue>;
    using JSONVariant = std::variant<std::monostate, Bool, Int, Float, String, JSONList, JSONDict>;
    struct JSONValue : JSONVariant { using JSONVariant::variant; };
}
using JSON = detail::JSONValue;

class JSONParser {
public:
    static JSON parse(const String& json) { size_t idx = 0; return parse_value(json, idx); }
    static JSON load_file(const String& path) {
        std::ifstream file(path);
        if (!file) return JSON{};
        std::stringstream buf;
        buf << file.rdbuf();
        return parse(buf.str());
    }
private:
    static void skip_ws(const String& s, size_t& idx) {
        while (idx < s.size() && std::isspace(s[idx])) ++idx;
    }
    static String parse_string(const String& s, size_t& idx) {
        ++idx;
        String result;
        while (idx < s.size() && s[idx] != '"') {
            if (s[idx] == '\\') {
                ++idx;
                if (idx >= s.size()) break;
                switch (s[idx]) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += s[idx]; break;
                }
            } else { result += s[idx]; }
            ++idx;
        }
        ++idx;
        return result;
    }
    static JSON parse_value(const String& s, size_t& idx);
};

inline JSON JSONParser::parse_value(const String& s, size_t& idx) {
    skip_ws(s, idx);
    if (idx >= s.size()) return JSON{std::monostate{}};
    char c = s[idx];
    if (c == 'n') { if (s.substr(idx,4)=="null") { idx+=4; return JSON{std::monostate{}}; } else throw std::runtime_error("Invalid JSON"); }
    if (c == 't') { if (s.substr(idx,4)=="true") { idx+=4; return JSON{true}; } else throw std::runtime_error("Invalid JSON"); }
    if (c == 'f') { if (s.substr(idx,5)=="false") { idx+=5; return JSON{false}; } else throw std::runtime_error("Invalid JSON"); }
    if (c == '"') return JSON{parse_string(s, idx)};
    if (c == '[') {
        idx++;
        detail::JSONList arr;
        while (true) {
            skip_ws(s, idx);
            if (idx >= s.size()) break;
            if (s[idx] == ']') { idx++; break; }
            arr.push_back(parse_value(s, idx));
            skip_ws(s, idx);
            if (s[idx] == ',') idx++;
        }
        return JSON{std::move(arr)};
    }
    if (c == '{') {
        idx++;
        detail::JSONDict obj;
        while (true) {
            skip_ws(s, idx);
            if (idx >= s.size()) break;
            if (s[idx] == '}') { idx++; break; }
            String key = parse_string(s, idx);
            skip_ws(s, idx);
            if (s[idx] == ':') idx++;
            obj[std::move(key)] = parse_value(s, idx);
            skip_ws(s, idx);
            if (s[idx] == ',') idx++;
        }
        return JSON{std::move(obj)};
    }
    // Number
    size_t start = idx;
    if (c == '-') idx++;
    while (idx < s.size() && std::isdigit(s[idx])) idx++;
    bool is_float = false;
    if (idx < s.size() && s[idx] == '.') {
        is_float = true; idx++;
        while (idx < s.size() && std::isdigit(s[idx])) idx++;
    }
    if (idx < s.size() && (s[idx]=='e' || s[idx]=='E')) {
        is_float = true; idx++;
        if (idx < s.size() && (s[idx]=='+' || s[idx]=='-')) idx++;
        while (idx < s.size() && std::isdigit(s[idx])) idx++;
    }
    String num_str = s.substr(start, idx-start);
    if (is_float) {
        Float val;
        auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data()+num_str.size(), val);
        if (ec == std::errc()) return JSON{val};
        throw std::runtime_error("Invalid number");
    } else {
        Int val;
        auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data()+num_str.size(), val);
        if (ec == std::errc()) return JSON{val};
        throw std::runtime_error("Invalid number");
    }
}

// ============================================================================
// 8. PRINT ENGINE (fully polymorphic, safe for all types)
// ============================================================================
struct end {
    String val;
    explicit end(const char* s) : val(s) {}
    explicit end(String s) : val(std::move(s)) {}
};

namespace detail {
    // --- type traits ---
    template <typename, typename = void> struct has_begin_end : std::false_type {};
    template <typename T> struct has_begin_end<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::true_type {};

    template <typename T> struct is_pair : std::false_type {};
    template <typename T, typename U> struct is_pair<std::pair<T, U>> : std::true_type {};

    template <typename T> struct is_tuple : std::false_type {};
    template <typename... Args> struct is_tuple<std::tuple<Args...>> : std::true_type {};

    template <typename T> struct is_optional : std::false_type {};
    template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

    template <typename T> struct is_end_tag : std::false_type {};
    template <> struct is_end_tag<end> : std::true_type {};

    // --- forward declarations ---
    template <typename T> void print_element(const T& val);
    void print_json(const JSON& j);

    // --- tuple printing ---
    template<typename T, std::size_t... Is>
    void print_tuple(const T& t, std::index_sequence<Is...>) {
        std::print("(");
        (..., (std::print("{}", Is==0?"":", "), print_element(std::get<Is>(t))));
        std::print(")");
    }

    // --- main polymorphic printer ---
    template <typename T>
    void print_element(const T& val) {
        if constexpr (std::is_same_v<T, Bool>) {
            std::print("{}", val ? "true" : "false");
        } else if constexpr (std::is_same_v<T, CommandResult>) {
            std::print("{}", val.output);
        } else if constexpr (std::is_same_v<T, std::monostate> || std::is_same_v<T, std::nullopt_t>) {
            std::print("None");
        } else if constexpr (is_optional<T>::value) {
            if (val.has_value()) {
                print_element(*val);   // recurse
            } else {
                std::print("None");
            }
        } else if constexpr (std::is_convertible_v<T, String>) {
            std::print("{}", val);
        } else if constexpr (std::is_same_v<T, JSON>) {
            print_json(val);
        } else if constexpr (is_pair<T>::value) {
            print_element(val.first);
            std::print(": ");
            print_element(val.second);
        } else if constexpr (is_tuple<T>::value) {
            print_tuple(val, std::make_index_sequence<std::tuple_size_v<T>>{});
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
            // fallback: try to format (for basic types like int, double, etc.)
            std::print("{}", val);
        }
    }

    void print_json(const JSON& j) {
        std::visit([](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::print("null");
            } else if constexpr (std::is_same_v<T, Bool>) {
                std::print("{}", arg ? "true" : "false");
            } else if constexpr (std::is_same_v<T, Int>) {
                std::print("{}", arg);
            } else if constexpr (std::is_same_v<T, Float>) {
                std::print("{}", arg);
            } else if constexpr (std::is_same_v<T, String>) {
                std::print("\"{}\"", arg);
            } else if constexpr (std::is_same_v<T, detail::JSONList>) {
                std::print("[");
                bool first = true;
                for (const auto& item : arg) {
                    if (!first) std::print(", ");
                    print_json(item);
                    first = false;
                }
                std::print("]");
            } else if constexpr (std::is_same_v<T, detail::JSONDict>) {
                std::print("{{");
                bool first = true;
                for (const auto& [k, v] : arg) {
                    if (!first) std::print(", ");
                    std::print("\"{}\": ", k);
                    print_json(v);
                    first = false;
                }
                std::print("}}");
            }
        }, j);
    }
}

// --- the public print() function ---
template <typename... Args>
void print(Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        std::print("\n");
    } else {
        constexpr size_t N = sizeof...(args);
        auto tup = std::forward_as_tuple(std::forward<Args>(args)...);
        if constexpr (detail::is_end_tag<std::decay_t<decltype(std::get<N-1>(tup))>>::value) {
            // end tag present → print first N-1 arguments, then end.val
            [&]<size_t... I>(std::index_sequence<I...>) {
                bool first = true;
                ((first ? (void)(first = false)
                        : (void)(std::print(" ")),
                  detail::print_element(std::get<I>(tup))), ...);
            }(std::make_index_sequence<N-1>{});
            std::print("{}", std::get<N-1>(tup).val);
            std::cout.flush();
        } else {
            // no end tag → print all arguments, then newline
            [&]<size_t... I>(std::index_sequence<I...>) {
                bool first = true;
                ((first ? (void)(first = false)
                        : (void)(std::print(" ")),
                  detail::print_element(std::get<I>(tup))), ...);
            }(std::make_index_sequence<N>{});
            std::print("\n");
        }
    }
}

// --- input / len ---
inline String input(const String& prompt = "") {
    if (!prompt.empty()) { std::print("{}", prompt); std::cout.flush(); }
    String line;
    std::getline(std::cin, line);
    return line;
}

template <typename C>
inline Int len(const C& container) { return static_cast<Int>(std::ranges::size(container)); }
inline Int len(const char* str) { return static_cast<Int>(std::strlen(str)); }

// ============================================================================
// 9. LOOP HELPERS
// ============================================================================
template <typename F>
inline void times(Int iterations, F&& lambda) {
    for (Int i = 0; i < iterations; ++i)
        std::forward<F>(lambda)(i);
}
template <typename C, typename B>
inline void while_loop(C&& condition, B&& body) {
    while (std::forward<C>(condition)())
        std::forward<B>(body)();
}

// ============================================================================
// 10. FILE I/O
// ============================================================================
inline String read_file(const String& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}
inline Bool write_file(const String& path, const String& content) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;
    file << content;
    return true;
}

// ============================================================================
// 11. FACTORIES
// ============================================================================
template <typename K, typename V>
inline auto pair(K&& k, V&& v) { return std::make_pair(std::forward<K>(k), std::forward<V>(v)); }

template <typename T>
inline List<T> klist(std::initializer_list<T> init) { return List<T>(init.begin(), init.end()); }
template <typename... Args> requires (sizeof...(Args) > 0)
inline auto klist(Args&&... args) {
    using T = std::common_type_t<std::decay_t<Args>...>;
    List<T> v; v.reserve(sizeof...(args));
    (v.push_back(std::forward<Args>(args)), ...);
    return v;
}
inline List<Int> klist() { return {}; }

template <typename... Args> requires (sizeof...(Args) > 0)
inline auto kdict(Args&&... args) {
    using First = std::tuple_element_t<0, std::tuple<std::decay_t<Args>...>>;
    using K = typename First::first_type;
    using V = typename First::second_type;
    Dict<K, V> m; m.reserve(sizeof...(args));
    (m.insert(std::forward<Args>(args)), ...);
    return m;
}
template <typename K, typename V>
inline Dict<K, V> kdict(std::initializer_list<std::pair<const K, V>> init) {
    return Dict<K, V>(init.begin(), init.end());
}
inline Dict<String, String> kdict() { return {}; }
            
template <typename... Args>
inline auto ktup(Args&&... args) {
    return std::make_tuple(std::forward<Args>(args)...);
}
// ============================================================================
// 11.5 CONTAINER HELPERS (Python‑style)
// ============================================================================
template <typename C, typename V>
void append(C& container, V&& value) {
    container.push_back(std::forward<V>(value));
}

template <typename C, typename... Values>
void extend(C& container, Values&&... values) {
    (container.push_back(std::forward<Values>(values)), ...);
}

// ============================================================================
// 11.6 KDLIST – Python‑style dynamic list (heterogeneous, high‑performance)
// ============================================================================
class kdlist {
    std::vector<JSON> data_;

    // ------------------------------------------------------------------------
    // Helper: convert any value to JSON with integral types forced to Int
    // (except bool, which stays Bool)
    // Special case for kdlist -> convert to JSONList
    // ------------------------------------------------------------------------
    template<typename T>
    static JSON to_json(T&& val) {
        using Decayed = std::decay_t<T>;
        if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>) {
            return JSON{static_cast<Int>(val)};
        } else if constexpr (std::is_same_v<Decayed, kdlist>) {
            // Convert nested kdlist to JSONList
            return std::forward<T>(val).to_json();
        } else {
            return JSON{std::forward<T>(val)};
        }
    }

    // ------------------------------------------------------------------------
    // Sorting comparator (total order over all JSON types)
    // ------------------------------------------------------------------------
    static int compare_json(const JSON& a, const JSON& b) {
        int type_a = a.index();
        int type_b = b.index();
        if (type_a != type_b) return type_a - type_b;
        return std::visit([&](const auto& va, const auto& vb) -> int {
            using TA = std::decay_t<decltype(va)>;
            using TB = std::decay_t<decltype(vb)>;
            if constexpr (std::is_same_v<TA, TB>) {
                if constexpr (std::is_same_v<TA, std::monostate>) return 0;
                else if constexpr (std::is_same_v<TA, Bool>) return (va ? 1 : 0) - (vb ? 1 : 0);
                else if constexpr (std::is_same_v<TA, Int>) return (va < vb) ? -1 : (va > vb) ? 1 : 0;
                else if constexpr (std::is_same_v<TA, Float>) return (va < vb) ? -1 : (va > vb) ? 1 : 0;
                else if constexpr (std::is_same_v<TA, String>) return va.compare(vb);
                else if constexpr (std::is_same_v<TA, detail::JSONList>) {
                    size_t min_sz = std::min(va.size(), vb.size());
                    for (size_t i = 0; i < min_sz; ++i) {
                        int cmp = compare_json(va[i], vb[i]);
                        if (cmp != 0) return cmp;
                    }
                    return (va.size() < vb.size()) ? -1 : (va.size() > vb.size()) ? 1 : 0;
                }
                else if constexpr (std::is_same_v<TA, detail::JSONDict>) {
                    // Compare by size for simplicity; could be expanded.
                    return (va.size() < vb.size()) ? -1 : (va.size() > vb.size()) ? 1 : 0;
                }
            }
            return 0;
        }, a, b);
    }

public:
    // --- Constructors --------------------------------------------------------
    kdlist() = default;

    // Explicit initializer list for raw JSON values (advanced use)
    kdlist(std::initializer_list<JSON> init) : data_(init) {}

    // Variadic constructor – all integral types become Int, others forwarded
    template<typename... Args>
    kdlist(Args&&... args) {
        data_.reserve(sizeof...(args));
        (data_.emplace_back(to_json(std::forward<Args>(args))), ...);
    }

    // --- Convert this kdlist to a JSON (as JSONList) -------------------------
    JSON to_json() const {
        detail::JSONList list;
        list.reserve(data_.size());
        for (const auto& item : data_) {
            list.push_back(item);
        }
        return JSON{std::move(list)};
    }

    // --- Capacity ------------------------------------------------------------
    [[nodiscard]] size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    void reserve(size_t n) { data_.reserve(n); }

    // --- Element access ------------------------------------------------------
    JSON& operator[](size_t index)       { return data_[index]; }
    const JSON& operator[](size_t index) const { return data_[index]; }
    JSON& at(size_t index) { return data_.at(index); }
    const JSON& at(size_t index) const { return data_.at(index); }
    JSON& front() { return data_.front(); }
    const JSON& front() const { return data_.front(); }
    JSON& back() { return data_.back(); }
    const JSON& back() const { return data_.back(); }

    // --- Modifiers -----------------------------------------------------------
    void append(const JSON& value) { data_.push_back(value); }
    void append(JSON&& value) { data_.push_back(std::move(value)); }

    // Variadic append – uses the same conversion helper
    template<typename... Args>
    void append(Args&&... args) {
        data_.emplace_back(to_json(std::forward<Args>(args)...));
    }

    void extend(const kdlist& other) {
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
    }
    void extend(kdlist&& other) {
        data_.insert(data_.end(),
                     std::make_move_iterator(other.data_.begin()),
                     std::make_move_iterator(other.data_.end()));
    }
    template<typename Iter>
    void extend(Iter first, Iter last) {
        data_.insert(data_.end(), first, last);
    }

    void insert(size_t index, const JSON& value) {
        data_.insert(data_.begin() + index, value);
    }
    void insert(size_t index, JSON&& value) {
        data_.insert(data_.begin() + index, std::move(value));
    }

    JSON pop() {
        if (empty()) throw std::out_of_range("pop from empty kdlist");
        JSON val = std::move(data_.back());
        data_.pop_back();
        return val;
    }
    JSON pop(size_t index) {
        if (index >= data_.size()) throw std::out_of_range("pop index out of range");
        JSON val = std::move(data_[index]);
        data_.erase(data_.begin() + index);
        return val;
    }

    void remove(const JSON& value) {
        auto it = std::find(data_.begin(), data_.end(), value);
        if (it == data_.end()) throw std::runtime_error("value not found in kdlist");
        data_.erase(it);
    }

    // --- Search --------------------------------------------------------------
    [[nodiscard]] int index(const JSON& value) const {
        auto it = std::find(data_.begin(), data_.end(), value);
        if (it == data_.end()) throw std::runtime_error("value not found in kdlist");
        return static_cast<int>(it - data_.begin());
    }
    [[nodiscard]] int count(const JSON& value) const {
        return static_cast<int>(std::count(data_.begin(), data_.end(), value));
    }

    // --- Ordering ------------------------------------------------------------
    void reverse() { std::reverse(data_.begin(), data_.end()); }
    void sort() {
        std::sort(data_.begin(), data_.end(),
                  [](const JSON& a, const JSON& b) { return compare_json(a, b) < 0; });
    }

    // --- Concatenation -------------------------------------------------------
    kdlist operator+(const kdlist& other) const {
        kdlist result = *this;
        result.extend(other);
        return result;
    }
    kdlist& operator+=(const kdlist& other) {
        extend(other);
        return *this;
    }
    kdlist& operator+=(kdlist&& other) {
        extend(std::move(other));
        return *this;
    }

    // --- Iterators -----------------------------------------------------------
    auto begin()       { return data_.begin(); }
    auto begin() const { return data_.begin(); }
    auto end()         { return data_.end(); }
    auto end()   const { return data_.end(); }

    // --- Comparison ----------------------------------------------------------
    bool operator==(const kdlist& other) const { return data_ == other.data_; }
    bool operator!=(const kdlist& other) const { return !(*this == other); }

    // --- Utilities -----------------------------------------------------------
    void clear() noexcept { data_.clear(); }
    const std::vector<JSON>& vec() const noexcept { return data_; }
};

// ============================================================================
// 12. TIMER & BENCHMARK
// ============================================================================
class Timer {
    std::chrono::high_resolution_clock::time_point start_;
public:
    Timer() { reset(); }
    void reset() { start_ = std::chrono::high_resolution_clock::now(); }
    Float elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<Float, std::milli>(now - start_).count();
    }
    template <typename F>
    static void benchmark(const String& label, F&& block) {
        auto start = std::chrono::high_resolution_clock::now();
        std::forward<F>(block)();
        auto end = std::chrono::high_resolution_clock::now();
        Float ms = std::chrono::duration<Float, std::milli>(end - start).count();
        ken::print("[BENCHMARK]", label, ":", ms, "ms");
    }
};

// ============================================================================
// 13. BASH PIPE OPERATOR
// ============================================================================
struct BashPipe {};
inline BashPipe bash;
inline CommandResult operator|(const String& cmd, [[maybe_unused]] BashPipe b) { return os.execute(cmd); }
inline CommandResult operator|(const char* cmd, [[maybe_unused]] BashPipe b) { return os.execute(String(cmd)); }

} // namespace ken

// ============================================================================
// 14. GLOBAL SHORTCUTS (guarded)
// ============================================================================
#ifndef var
#  define var auto
#endif
#ifndef func
#  define func auto
#endif
#ifndef def
#  define def auto
#endif
#ifndef MAIN
#define MAIN int main(int __argc__,char* __args__[])
#endif

#ifndef KEN_NO_GLOBAL_NAMESPACE
using namespace ken;
#endif

#endif // KEN_HPP
