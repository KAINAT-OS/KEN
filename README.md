# 🎌 Ken Language — Complete Documentation

**Ken** is a single-header C++23 library that wraps the raw power of C++ in a syntax so clean it feels like writing Python. Zero-cost abstractions mean you get high-level expressiveness that compiles straight to bare-metal machine code. No garbage collection, no runtime overhead — just pure speed.

---

## 📖 Table of Contents

1. [Quick Start](#quick-start)
2. [Core Philosophy](#core-philosophy)
3. [Installation](#installation)
4. [The Golden Brace Rule](#the-golden-brace-rule) ⭐ **IMPORTANT**
5. [Type System](#type-system)
6. [Variables & Type Deduction](#variables--type-deduction)
7. [Containers](#containers)
   - [Lists](#lists)
   - [Kdlists](#kdlists)
   - [Dicts](#dicts)
   - [Sets](#sets)
   - [Tuples](#tuples)
8. [The Print Engine](#the-print-engine)
9. [Loops & Iteration](#loops--iteration)
10. [Functional Programming](#functional-programming)
11. [Slicing & Span](#slicing--span)
12. [String Operations](#string-operations)
13. [File I/O](#file-io)
14. [JSON Handling](#json-handling)
15. [Operating System Interface](#operating-system-interface)
16. [Timing & Benchmarking](#timing--benchmarking)
17. [Factory Functions](#factory-functions)
18. [Input](#input)
19. [The Kencc Compiler](#the-kencc-compiler)
20. [Configuration Macros](#configuration-macros)
21. [Complete API Reference](#complete-api-reference)
22. [Example Programs](#example-programs)
23. [Requirements](#requirements)
24. [Design Principles](#design-principles)

---

## Quick Start

Create a file `hello.ken`:

```cpp
#include "ken.hpp"

def main() -> int {
    print("Hello, World!")
    return 0
}
```

Compile and run:

```bash
kencc hello.ken -o hello
./hello
```

**That's it.** No semicolons, no boilerplate, no build system.

---

## Core Philosophy

| Python Feature | Ken Equivalent | Overhead |
|---------------|----------------|----------|
| `str` | `String` | None (alias) |
| `int` | `Int` (int64_t) | None |
| `float` | `Float` (double) | None |
| `bool` | `Bool` | None |
| `list` | `List<T>` (vector) | None |
| `dict` | `Dict<K,V>` (unordered_map) | None |
| `tuple` | `Tuple<Ts...>` | None |
| `set` | `Set<T>` (unordered_set) | None |
| `None` | `None` (nullopt) | None |
| `range()` | `range()` (lazy iterator) | **Zero memory** |
| `map()` | `map()` (lazy view) | **Compiles to loop** |
| `filter()` | `filter()` (lazy view) | **Compiles to loop** |
| `print()` | `print()` | Minimal |
| `input()` | `input()` | Minimal |
| `len()` | `len()` | None |
| `append()` | `append()` | None |
| `extend()` | `extend()` | None |

Every high-level feature in Ken compiles to the exact same assembly as hand-written C++. No virtual functions, no heap allocations (unless the container itself allocates), no hidden overhead.

---

## Installation

### System-Wide Installation

```bash
git clone https://github.com/yourusername/ken.git
cd ken
sudo ./install.sh
```

This installs:
- `/usr/local/include/ken.hpp` — the header
- `/usr/local/bin/ken_prep` — semicolon preprocessor
- `/usr/local/bin/kencc` — compiler driver

### Manual Installation

```bash
# Install the header
sudo cp ken.hpp /usr/local/include/

# Compile and install the preprocessor
g++ -std=c++23 -O3 ken_prep.cpp -o /usr/local/bin/ken_prep

# Install the compiler wrapper
sudo cp kencc /usr/local/bin/kencc
sudo chmod +x /usr/local/bin/kencc
```

### Requirements

- **Compiler**: GCC 14+ or Clang 18+ with full C++23 support
- **Build flag**: `-std=c++23` (handled automatically by `kencc`)
- **No external dependencies** — everything is in the C++ standard library
- **Header-only** — just `#include "ken.hpp"`

---

## The Golden Brace Rule ⭐

> **"Only functions and control flow use `{ }` — everything else uses `( )`."**

Ken's semicolon‑free magic works best when you follow one simple rule:

### ✅ **Use `( )` for initialisation and calls**
```cpp
var nums = kdlist(1, 2, 3)        // ✅ Good – uses parentheses
var name = String("Alice")        // ✅ Good
var result = klist(42, "hello")   // ✅ Good
```

### ✅ **Use `{ }` only for function/block bodies**
```cpp
MAIN {                             // ✅ Good – MAIN is a function
    print("Hello")
    if (x > 0) {                   // ✅ Good – control flow block
        print("Positive")
    }
}

def calculate() -> Int {           // ✅ Good – function body
    return 42
}
```

### ❌ **Avoid `{ }` for assignments and initialisation**
```cpp
var nums = kdlist{1, 2, 3}        // ❌ Avoid – can confuse the preprocessor
var name = String{"Alice"}        // ❌ Avoid
var result = klist{42, "hello"}   // ❌ Avoid
```

### Why?

The `ken_prep` preprocessor automatically inserts semicolons at the end of lines. But when a line ends with `}` (like `kdlist{1, 2, 3}`), the preprocessor thinks the statement is already terminated and **does not add a semicolon**. This leads to confusing compiler errors.

By using `(` instead of `{` for initialisation, you always get clean, predictable semicolon insertion.

**Exception:** Raw C++ initialiser lists (like `List<Int>{1, 2, 3}`) are fine because they're part of the underlying C++ syntax and you'll likely add explicit semicolons when mixing raw C++ with Ken.

---

## Type System

### Basic Types

```cpp
String name = "Alice"     // std::string
Int age = 42              // int64_t
Float pi = 3.14159        // double
Bool is_active = true     // bool
```

All Ken types are type aliases for standard C++ types — zero overhead, maximum compatibility.

### Container Types

```cpp
// Lists (dynamic arrays)
List<Int> numbers = {1, 2, 3, 4, 5}
List<String> names = klist("Alice", "Bob", "Charlie")

// Dicts (hash maps)
Dict<String, Int> ages = kdict(
    pair("Alice", 30),
    pair("Bob", 25)
)

// Sets
Set<Int> unique_numbers = {1, 2, 3}

// Tuples
Tuple<Int, String, Bool> person = {42, "Alice", true}
```

### None

```cpp
var x = None              // std::nullopt
std::optional<Int> y = None
```

---

## Variables & Type Deduction

Ken provides three macros for type deduction, all expanding to `auto`:

```cpp
var name = "Alice"        // auto name = "Alice";
func get_value() {        // auto get_value()
    return 42
}
def my_function() {       // auto my_function()
    return "hello"
}
```

- `var` — for variables (like Python, Rust, Swift)
- `func` — for functions (C++ style)
- `def` — for functions (Python style)

They all do exactly the same thing. Use whichever feels natural.

### The MAIN Macro

For Python-style entry points without semicolons:

```cpp
MAIN {
    print("Hello from MAIN!")
    return 0
}
```

This expands to `int main()`. Perfect for `.ken` files where you don't want to type `int main()`.

---

## Containers

### Lists

Lists are dynamic arrays (`std::vector<T>`):

```cpp
// Create lists
var nums = klist(1, 2, 3, 4, 5)
var names = klist("Alice", "Bob", "Charlie")
var empty = klist()                    // empty List<Int>

// Or directly
List<Float> temps = {98.6, 99.1, 100.2}

// Python-style append and extend
append(nums, 6)                        // adds single element
extend(nums, 7, 8, 9)                 // adds multiple elements

// Access elements
var first = nums[0]                    // 1
nums[0] = 99                           // modify in place

// Standard operations
var size = len(nums)                   // 9
nums.push_back(10)                     // C++ style also works
```

---

### Kdlists

**`kdlist`** is Ken's dynamic, **heterogeneous** list type — a full clone of Python's `list`. It can hold values of any type: integers, floats, strings, booleans, nested lists, dictionaries, or anything that can be represented as a `JSON` value. Under the hood it stores `JSON` variants in a `std::vector`, giving you `O(1)` amortized access, append, and pop while maintaining type safety at runtime.

```cpp
var mixed = kdlist("a", 1, 1.4, true, kdlist(42, "nested"))
print(mixed)
// ["a", 1, 1.4, true, [42, "nested"]]
```

#### Creating a `kdlist`

| Constructor                          | Description |
|--------------------------------------|-------------|
| `kdlist()`                           | Empty list |
| `kdlist(args...)`                    | Variadic constructor – each argument becomes an element |

```cpp
auto empty  = kdlist()                       // []
auto nums   = kdlist(1, 2, 3)                // [1, 2, 3]
auto mixed  = kdlist("hello", 42, 3.14)      // ["hello", 42, 3.14]
auto nested = kdlist(1, kdlist("inner"))     // [1, ["inner"]]
```

> 💡 **Remember:** Always use `( )` for `kdlist` construction, not `{ }`. This ensures the preprocessor inserts semicolons correctly.

#### Accessing Elements

| Method                 | Description |
|------------------------|-------------|
| `operator[](size_t)`   | Bounds‑unchecked access (fast) |
| `at(size_t)`           | Bounds‑checked access (throws `std::out_of_range`) |
| `front()` / `back()`   | First/last element |

```cpp
auto lst = kdlist(10, 20, 30)
print(lst[0])          // 10
print(lst.at(1))       // 20
print(lst.front())     // 10
print(lst.back())      // 30
```

#### Modifiers

| Method                          | Description |
|---------------------------------|-------------|
| `append(value)`                 | Add a single element at the end |
| `extend(other)` / `extend(first, last)` | Append all elements from another list or iterator range |
| `insert(index, value)`          | Insert value at given index |
| `pop()` / `pop(index)`          | Remove and return last element (or at index) |
| `remove(value)`                 | Remove first occurrence of value (throws if not found) |
| `clear()`                       | Remove all elements |
| `reserve(size)`                 | Pre‑allocate capacity for performance |

```cpp
auto lst = kdlist(1, 2, 3)

lst.append(4)                 // [1, 2, 3, 4]
lst.insert(1, 99)             // [1, 99, 2, 3, 4]

auto other = kdlist(5, 6)
lst.extend(other)             // [1, 99, 2, 3, 4, 5, 6]

var last = lst.pop()          // returns 6, list becomes [1, 99, 2, 3, 4, 5]
var second = lst.pop(1)       // returns 99, list becomes [1, 2, 3, 4, 5]

lst.remove(3)                 // [1, 2, 4, 5]
lst.clear()                   // []
```

#### Searching

| Method            | Description |
|-------------------|-------------|
| `index(value)`    | Return first index of value (throws if not found) |
| `count(value)`    | Return number of occurrences |

```cpp
auto lst = kdlist(1, 2, 3, 2, 1)
print(lst.index(2))   // 1
print(lst.count(2))   // 2
```

#### Ordering

| Method      | Description |
|-------------|-------------|
| `reverse()` | Reverse list in‑place |
| `sort()`    | Sort list in‑place using a deterministic total order over all JSON types |

**Sort order**:  
`null` < `false` < `true` < integers < floats < strings < arrays < objects.  
For equal types, lexicographical / numerical comparison is used.

```cpp
auto mixed = kdlist(3, 1, 4, 1.5, "b", "a", true, false)
mixed.sort()
print(mixed)   // [false, true, 1, 3, 4, 1.5, "a", "b"]

mixed.reverse()
print(mixed)   // ["b", "a", 1.5, 4, 3, 1, true, false]
```

#### Concatenation

| Operation               | Description |
|-------------------------|-------------|
| `lst + other`           | Returns a new `kdlist` combining both |
| `lst += other`          | Appends elements from `other` to `lst` (in‑place) |

```cpp
auto a = kdlist(1, 2)
auto b = kdlist(3, 4)
auto c = a + b        // [1, 2, 3, 4]
a += b                // a now [1, 2, 3, 4]
```

#### Iteration & Range‑based For

```cpp
auto lst = kdlist("apple", "banana", "cherry")
for (const auto& item : lst) {
    print(item)
}
```

You can also use STL algorithms with `begin()` / `end()`:

```cpp
auto lst = kdlist(10, 20, 30)
var sum = std::accumulate(lst.begin(), lst.end(), 0)
```

#### Comparison

`kdlist` supports `==` and `!=`, comparing element‑wise in order.

```cpp
auto a = kdlist(1, 2, 3)
auto b = kdlist(1, 2, 3)
auto c = kdlist(1, 2, 4)
print(a == b)   // true
print(a == c)   // false
```

#### Utility

| Method          | Description |
|-----------------|-------------|
| `size()`        | Number of elements |
| `empty()`       | Check if empty |
| `vec()`         | Return a const reference to the underlying `std::vector<JSON>` (for advanced use) |

#### Performance & Implementation

`kdlist` is a thin wrapper around `std::vector<JSON>`. Every operation is **inline** and compiles to the same machine code as using a vector directly — no virtual calls, no hidden overhead. The `JSON` variant is a stack‑based discriminated union, so elements are stored contiguously and access is cache‑friendly.

- **Append** is amortised O(1) – `std::vector`'s growth policy ensures good performance.
- **Insert / erase** at arbitrary positions is O(n) (same as `std::vector`).
- **Sort** uses `std::sort` with a custom comparator; complexity O(n log n).

If you need a homogeneous, statically‑typed list, prefer `List<T>` (plain `std::vector`) – it has even less overhead because it doesn't carry a variant tag. Use `kdlist` only when you truly need heterogeneity.

#### Example: Processing Heterogeneous Data

```cpp
MAIN {
    var records = kdlist(
        kdlist("Alice", 30, true),
        kdlist("Bob", 25, false),
        kdlist("Charlie", 35, true)
    )

    // Filter and transform
    for (var record : records) {
        var name  = record[0]
        var age   = record[1]
        var active = record[2]
        if (active) {
            print(name, "is active and", age, "years old")
        }
    }

    // Add a new record
    records.append(kdlist("Diana", 28, true))
    print(records)
}
```

#### Comparison with `List<T>`

| Feature | `List<T>` (`std::vector`) | `kdlist` |
|---------|---------------------------|----------|
| Element type | Homogeneous (compile‑time) | Heterogeneous (runtime) |
| Type safety | Full compile‑time | Runtime checks (variant) |
| Memory footprint | Minimal (exact size of T) | Larger (variant overhead) |
| Use case | Performance‑critical, same‑type data | Flexible, mixed‑type data (JSON‑like) |

Choose the right tool for the job: `List<T>` for speed, `kdlist` for flexibility.

---

**Next:** [Dicts](#dicts)

### Dicts

Dicts are hash maps (`std::unordered_map<K, V>`):

```cpp
// Create dicts
var scores = kdict(
    pair("Alice", 95),
    pair("Bob", 87),
    pair("Charlie", 92)
)

// Access and modify
scores["Alice"] = 97                   // update value
var alice_score = scores["Alice"]      // read value

// Check existence (using C++ methods)
if (scores.contains("Dave")) {
    print("Dave found")
}

// Iterate
for (var [key, value] : scores) {
    print(key, ":", value)
}
```

### Sets

Sets are unordered unique collections (`std::unordered_set<T>`):

```cpp
Set<Int> unique_nums = {1, 2, 3, 3, 2, 1}   // becomes {1, 2, 3}
unique_nums.insert(4)
```

### Tuples

Tuples are fixed-size heterogeneous collections:

```cpp
var person = std::make_tuple(42, "Alice", true)
var id = std::get<0>(person)
var name = std::get<1>(person)
```

Ken provides `ktup()` as a shortcut:

```cpp
var name = String("Alice")
var age = Int(30)
var scores = klist(95, 87, 92)
var t2 = ktup(name, age, scores)
```

---

## The Print Engine

### Basic Printing

```cpp
print("Hello, World!")
print("The answer is", 42)
print("Pi is approximately", 3.14)
```

### Custom Line Endings

```cpp
print("Loading...", end(""))
print(" Done!")
// Output: Loading... Done!

print("Item 1", end(" | "))
print("Item 2")
// Output: Item 1 | Item 2
```

### Printing Complex Types

The print engine automatically handles:

- **Booleans**: `true` / `false` (not `1` / `0`)
- **Optional/None**: Prints `None`
- **Pairs**: `key: value`
- **Tuples**: `(item1, item2, ...)`
- **Vectors/Lists**: `[item1, item2, ...]`
- **Maps/Dicts**: `key1: val1, key2: val2`
- **JSON**: Pretty-printed recursively

```cpp
var data = klist(1, 2, 3)
print(data)                      // [1, 2, 3]

var person = pair("name", "Bob")
print(person)                    // name: Bob

var coords = std::make_tuple(10, 20, 30)
print(coords)                    // (10, 20, 30)

std::optional<Int> maybe = 42
print(maybe)                     // 42
print(std::optional<Int>())      // None
```

---

## Loops & Iteration

### Range-Based Loops (Zero Allocation!)

The `range()` function returns a **lazy iterator** — it generates values on-the-fly with zero memory allocation. A loop over `range(1'000'000'000)` uses no extra memory.

```cpp
// Python: for i in range(10):
for (var i : range(10)) {
    print(i)
}

// With start and stop
for (var i : range(5, 15)) {
    print(i)
}

// With step
for (var i : range(0, 10, 2)) {
    print(i)   // 0, 2, 4, 6, 8
}

// Negative step
for (var i : range(10, 0, -1)) {
    print(i)   // 10, 9, 8, ..., 1
}
```

### `times()` — Execute N Times

```cpp
times(5, [](var i) {
    print("Iteration", i)
})
// Iteration 0
// Iteration 1
// Iteration 2
// Iteration 3
// Iteration 4
```

### `while_loop()` — While with Lambda

```cpp
var counter = 0
while_loop(
    [&]() { return counter < 5 },
    [&]() {
        print(counter)
        counter++
    }
)
```

### `enumerate()` — Index + Value

```cpp
var names = klist("Alice", "Bob", "Charlie")

for (var [i, name] : enumerate(names)) {
    print(i, ":", name)
}
// 0: Alice
// 1: Bob
// 2: Charlie
```

### `zip()` — Parallel Iteration

```cpp
var names = klist("Alice", "Bob")
var ages  = klist(30, 25)

for (var [name, age] : zip(names, ages)) {
    print(name, "is", age, "years old")
}
```

---

## Functional Programming

All functional operations return **lazy views** — they don't create new containers unless you explicitly collect them. They compile to direct loops with zero overhead.

### `map()` — Transform Elements

```cpp
var numbers = klist(1, 2, 3, 4)
var squares = map([](var x) { return x * x }, numbers)

for (var x : squares) {
    print(x)   // 1, 4, 9, 16
}
```

### `filter()` — Keep Matching Elements

```cpp
var numbers = klist(1, 2, 3, 4, 5, 6)
var evens = filter([](var x) { return x % 2 == 0 }, numbers)

for (var x : evens) {
    print(x)   // 2, 4, 6
}
```

### `reduce()` — Fold/Aggregate

```cpp
var numbers = klist(1, 2, 3, 4, 5)

// Sum with initial value
var sum = reduce(numbers, Int(0), [](var a, var b) { return a + b })
print(sum)   // 15

// Product without initial value
var product = reduce(numbers, [](var a, var b) { return a * b })
print(product)   // 120
```

### `any_of()` & `all_of()` — Predicate Checks

```cpp
var numbers = klist(1, 2, 3, 4, 5)

var has_even = any_of(numbers, [](var x) { return x % 2 == 0 })
print(has_even)   // true

var all_positive = all_of(numbers, [](var x) { return x > 0 })
print(all_positive)   // true
```

---

## Slicing & Span

### `slice()` — Extract Sub-Sequence

Returns a new `List` (explicit copy, just like Python's `[:]`):

```cpp
var numbers = klist(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)

var first_three = slice(numbers, 0, 3)
print(first_three)   // [0, 1, 2]

var last_three = slice(numbers, -3, len(numbers))
print(last_three)   // [7, 8, 9]

var every_other = slice(numbers, 0, 10, 2)
print(every_other)   // [0, 2, 4, 6, 8]

var reversed = slice(numbers, len(numbers) - 1, -1, -1)
print(reversed)   // [9, 8, 7, ..., 0]
```

### `span()` — Zero-Copy View

Returns a `std::span` referencing the original memory — no copy:

```cpp
var numbers = klist(0, 1, 2, 3, 4, 5)
var view = span(numbers, 2, 3)   // start=2, count=3
// view references numbers[2..5] directly
```

### `len()` — Size of Any Container

```cpp
var numbers = klist(1, 2, 3)
print(len(numbers))              // 3

var name = "Alice"
print(len(name))                 // 5

print(len("hello"))              // 5 (C-string overload)
```

---

## String Operations

All methods are static on the `StringUtils` class:

```cpp
using namespace ken
```

### Split & Join

```cpp
var csv = "apple,banana,cherry"
var parts = StringUtils::split(csv, ',')
print(parts)                     // [apple, banana, cherry]

var joined = StringUtils::join(parts, " | ")
print(joined)                    // apple | banana | cherry
```

### Search

```cpp
var text = "Hello, World!"
print(StringUtils::contains(text, "World"))    // true
print(StringUtils::contains(text, "xyz"))      // false
```

### Starts/Ends With

```cpp
var url = "https://example.com"
print(StringUtils::starts_with(url, "https"))  // true
print(StringUtils::ends_with(url, ".org"))     // false
```

### Case Conversion

```cpp
var text = "Hello, World!"
print(StringUtils::lower(text))   // hello, world!
print(StringUtils::upper(text))   // HELLO, WORLD!
```

### Strip Whitespace

```cpp
var messy = "   hello world   \n"
print(StringUtils::strip(messy))    // "hello world"
print(StringUtils::lstrip(messy))   // "hello world   \n"
print(StringUtils::rstrip(messy))   // "   hello world"
```

### Replace

```cpp
var text = "I like cats. Cats are great."
var replaced = StringUtils::replace(text, "cat", "dog")
print(replaced)   // I like dogs. Dogs are great.
```

---

## File I/O

### Read File

```cpp
var content = read_file("input.txt")
if (content.empty()) {
    print("File not found or empty")
} else {
    print(content)
}
```

### Write File

```cpp
var success = write_file("output.txt", "Hello, World!\n")
if (success) {
    print("File written successfully")
} else {
    print("Failed to write file")
}
```

---

## JSON Handling

### Parse JSON String

```cpp
var json_str = R"({
    "name": "Alice",
    "age": 30,
    "active": true,
    "tags": ["dev", "cpp"],
    "address": {
        "city": "Tokyo",
        "zip": "100-0001"
    }
})"

var data = JSONParser::parse(json_str)
print(data)
```

Output:
```
{"active": true, "address": {"city": "Tokyo", "zip": "100-0001"}, "age": 30, "name": "Alice", "tags": ["dev", "cpp"]}
```

### Parse JSON from File

```cpp
var config = JSONParser::load_file("config.json")
print(config)
```

### Access JSON Values with `std::visit`

```cpp
var json = JSONParser::parse(R"({"name": "Alice", "age": 30})")

std::visit([](var& val) {
    using T = std::decay_t<decltype(val)>
    
    if constexpr (std::is_same_v<T, String>) {
        print("String:", val)
    } else if constexpr (std::is_same_v<T, Int>) {
        print("Integer:", val)
    } else if constexpr (std::is_same_v<T, Float>) {
        print("Float:", val)
    } else if constexpr (std::is_same_v<T, Bool>) {
        print("Boolean:", val)
    } else if constexpr (std::is_same_v<T, detail::JSONList>) {
        print("Array with", len(val), "items")
    } else if constexpr (std::is_same_v<T, detail::JSONDict>) {
        print("Object with", len(val), "keys")
    }
}, json)
```

---

## Operating System Interface

The global `os` object provides system operations. It's a singleton — always available, no instantiation needed.

### Execute Shell Commands

```cpp
var result = os.execute("ls -la")
print(result.output)
print("Exit code:", result.exit_code)
```

### Bash Pipe Syntax

```cpp
// Python/bash style piping
var result = "uname -a" | bash
print(result)

// Access as string directly
String kernel_info = "uname -r" | bash
print(kernel_info)
```

### Environment Variables

```cpp
var home = os.getenv("HOME")
print("Home directory:", home)

var path = os.getenv("PATH")
print("Path:", path)
```

### Filesystem Operations

```cpp
// Check if path exists
if (os.path_exists("config.json")) {
    print("Config file found")
}

// List directory contents
var files = os.listdir(".")
for (var& file : files) {
    print(file)
}

// Change working directory
os.chdir("/tmp")

// Create directory
os.mkdir("new_folder")
```

### Delays & Exit

```cpp
os.delay_msec(500)       // Wait 500 milliseconds
os.delay_sec(2.5)        // Wait 2.5 seconds
os.exit(0)               // Exit program with code 0
```

---

## Timing & Benchmarking

### Timer

```cpp
Timer t
os.delay_msec(100)
print("Elapsed:", t.elapsed(), "ms")
```

### Benchmark Function

```cpp
Timer::benchmark("Heavy computation", []() {
    var sum = 0
    for (var i : range(1'000'000)) {
        sum += i
    }
    print("Sum:", sum)
})

// Output:
// Sum: 499999500000
// [BENCHMARK] Heavy computation : 2.3 ms
```

---

## Factory Functions

### `klist()` — Create Lists

```cpp
var empty = klist()                     // Empty list
var nums = klist(1, 2, 3, 4, 5)        // From values
var copy = klist({1, 2, 3})            // From initializer list
```

### `kdict()` — Create Dicts

```cpp
var empty = kdict()                     // Empty dict

var ages = kdict(
    pair("Alice", 30),
    pair("Bob", 25)
)

var from_initializer = kdict<String, Int>({
    {"Alice", 30},
    {"Bob", 25}
})
```

### `pair()` — Create Key-Value Pair

```cpp
var entry = pair("key", 42)
print(entry)   // key: 42
```

### `append()` — Add to Container (Python-style)

```cpp
List<Int> nums
append(nums, 10)
append(nums, 20)
print(nums)   // [10, 20]
```

### `extend()` — Add Multiple Elements

```cpp
List<Int> nums
extend(nums, 10, 20, 30, 40)
print(nums)   // [10, 20, 30, 40]
```

---

## Input

```cpp
// Simple input
var name = input("Enter your name: ")
print("Hello,", name)

// No prompt
var line = input()
print("You typed:", line)
```

---

## The Kencc Compiler

### What It Does

The `kencc` command:

1. Runs `ken_prep` to insert missing semicolons
2. Compiles with `g++ -std=c++23 -O2`
3. Produces your executable

### Usage

```bash
kencc myprogram.ken -o myprogram
./myprogram
```

Any extra flags are passed directly to `g++`:

```bash
kencc myprogram.ken -o myprogram -Wall -Wextra -O3
```

### What You Can Skip

In `.ken` files, `kencc` allows you to omit:
- **Semicolons** — automatically inserted
- **`int main()`** — use `MAIN { }` instead

---

## Configuration Macros

### `KEN_NO_GLOBAL_NAMESPACE`

Define this **before** including ken.hpp to avoid polluting the global namespace:

```cpp
#define KEN_NO_GLOBAL_NAMESPACE
#include "ken.hpp"

// Now you must use ken:: prefix
ken::print("Hello")
ken::String name = "Alice"
```

### Macro Override Protection

The macros `var`, `func`, `def`, and `MAIN` are guarded. If you've already defined them, ken.hpp won't override:

```cpp
#define var my_custom_var
#include "ken.hpp"
// var still means my_custom_var
```

---

## Complete API Reference

### Type Aliases

| Alias | Real Type | Description |
|-------|-----------|-------------|
| `String` | `std::string` | Text string |
| `Int` | `int64_t` | 64-bit signed integer |
| `Float` | `double` | Double-precision float |
| `Bool` | `bool` | Boolean |
| `List<T>` | `std::vector<T>` | Dynamic array |
| `Dict<K,V>` | `std::unordered_map<K,V>` | Hash map |
| `Set<T>` | `std::unordered_set<T>` | Hash set |
| `Tuple<Ts...>` | `std::tuple<Ts...>` | Fixed tuple |
| `Span<T>` | `std::span<T>` | Non-owning view |
| `None` | `std::nullopt` | Null optional |

### Functions

| Function | Description |
|----------|-------------|
| `print(args..., end("\n"))` | Formatted output with optional custom ending |
| `input(prompt="")` | Read line from stdin |
| `range(stop)` | Lazy sequence 0..stop-1 |
| `range(start, stop, step)` | Lazy sequence with full control |
| `len(container)` | Size of any container or string |
| `times(n, lambda)` | Execute lambda n times |
| `while_loop(cond, body)` | While loop with lambdas |
| `enumerate(range)` | Index + value iteration |
| `zip(ranges...)` | Parallel iteration |
| `map(func, range)` | Transform elements (lazy) |
| `filter(pred, range)` | Keep matching elements (lazy) |
| `reduce(range, init, op)` | Fold/aggregate |
| `any_of(range, pred)` | Check if any element matches |
| `all_of(range, pred)` | Check if all elements match |
| `slice(list, start, stop, step)` | Extract sub-sequence |
| `span(list, start, count)` | Zero-copy contiguous view |
| `read_file(path)` | Read entire file to string |
| `write_file(path, content)` | Write string to file |
| `klist(args...)` | Create list |
| `kdict(pairs...)` | Create dict |
| `pair(key, value)` | Create key-value pair |
| `append(container, value)` | Add element to container |
| `extend(container, values...)` | Add multiple elements |

### OS Object Methods

| Method | Description |
|--------|-------------|
| `os.execute(cmd)` | Run shell command, return output+exit code |
| `os.getenv(name)` | Get environment variable |
| `os.path_exists(path)` | Check if filesystem path exists |
| `os.listdir(path)` | List directory contents |
| `os.chdir(path)` | Change working directory |
| `os.mkdir(path)` | Create directory |
| `os.delay_msec(ms)` | Sleep milliseconds |
| `os.delay_sec(s)` | Sleep seconds |
| `os.exit(code)` | Exit program |

### StringUtils Methods

| Method | Description |
|--------|-------------|
| `split(str, delim)` | Split string by delimiter |
| `join(list, delim)` | Join strings with delimiter |
| `contains(str, substr)` | Check if string contains substring |
| `starts_with(str, prefix)` | Check prefix |
| `ends_with(str, suffix)` | Check suffix |
| `lower(str)` | Convert to lowercase |
| `upper(str)` | Convert to uppercase |
| `strip(str)` | Remove leading/trailing whitespace |
| `lstrip(str)` | Remove leading whitespace |
| `rstrip(str)` | Remove trailing whitespace |
| `replace(str, from, to)` | Replace all occurrences |

### JSONParser Methods

| Method | Description |
|--------|-------------|
| `parse(json_string)` | Parse JSON string to `JSON` type |
| `load_file(path)` | Parse JSON file to `JSON` type |

### Timer Methods

| Method | Description |
|--------|-------------|
| `timer.elapsed()` | Get elapsed milliseconds |
| `timer.reset()` | Reset timer |
| `Timer::benchmark(label, lambda)` | Time and print function execution |

### Macros

| Macro | Expands To | Purpose |
|-------|-----------|---------|
| `var` | `auto` | Variable type deduction |
| `func` | `auto` | Function return type deduction |
| `def` | `auto` | Function return type deduction (Python style) |
| `MAIN` | `int main()` | Clean entry point |

---

## Example Programs

### FizzBuzz

```cpp
#include "ken.hpp"

def main() -> int {
    for (var i : range(1, 101)) {
        if (i % 15 == 0)      print("FizzBuzz")
        else if (i % 3 == 0)  print("Fizz")
        else if (i % 5 == 0)  print("Buzz")
        else                  print(i)
    }
    return 0
}
```

### Word Counter

```cpp
#include "ken.hpp"

MAIN {
    var text = read_file("input.txt")
    var words = StringUtils::split(text, ' ')
    
    Dict<String, Int> counter
    for (var& word : words) {
        var cleaned = StringUtils::strip(word)
        if (!cleaned.empty()) {
            counter[StringUtils::lower(cleaned)]++
        }
    }
    
    for (var [word, count] : counter) {
        print(word, ":", count)
    }
    return 0
}
```

### Web API Fetcher

```cpp
#include "ken.hpp"

MAIN {
    var response = "curl -s https://api.github.com/users/octocat" | bash
    var user_data = JSONParser::parse(response)
    print(user_data)
    return 0
}
```

### File Watcher

```cpp
#include "ken.hpp"

MAIN {
    var watched_file = "data.txt"
    String last_content
    
    while_loop([&]() { return true }, [&]() {
        var current = read_file(watched_file)
        if (current != last_content) {
            print("File changed!")
            print(current)
            last_content = current
        }
        os.delay_sec(1.0)
    })
    return 0
}
```

### Quick Benchmark

```cpp
#include "ken.hpp"

MAIN {
    var data = klist()
    for (var i : range(1'000'000)) {
        append(data, i)
    }
    
    Timer::benchmark("Map + Filter + Reduce", [&]() {
        var result = reduce(
            filter(
                [](var x) { return x % 2 == 0 },
                map(
                    [](var x) { return x * x },
                    data
                )
            ),
            Int(0),
            [](var a, var b) { return a + b }
        )
        print("Result:", result)
    })
    return 0
}
```

---

## Design Principles

1. **Zero Cost** — Abstractions compile away completely. `range(10)` becomes `for(int i=0;i<10;++i)`.
2. **Familiar** — Syntax mirrors Python, Java, and Bash where possible. Python developers feel at home.
3. **Progressive** — Use simple features first, advanced features when ready. You can always drop to raw C++.
4. **Non-invasive** — Macros are guarded, global namespace is optional. Won't break existing code.
5. **Batteries included** — Common operations are built in. No dependency hunting.
6. **Semicolon-free** — The `kencc` preprocessor handles semicolons so you don't have to, but remember the Golden Brace Rule!
7. **Header-only** — No linking, no build system required. Just `#include "ken.hpp"`.

---

**Ken** — *When you want Python's clarity with C++'s fury.* 🎌
