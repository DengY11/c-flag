# C++ Flag Parsing Library

This is a lightweight, header-only C++ library for parsing command-line flags.

## Features

- Supports `int64_t`, `double`, `bool`, and `std::string` flag types.
- Parses long options (e.g., `--port 9090`, `--port=9090`).
- Handles boolean flags without explicit values (e.g., `--debug` implies `true`).
- Supports positional arguments.
- Provides clear error messages for unknown flags, missing values, and invalid values.
- Includes a `Get<T>` helper function for easy, type-safe value retrieval.

## Usage

To use the library, simply include the `cppflag.hpp` header file in your project.

### 1. Defining Flags

First, create a `FlagSet` object. Then, define your flags using the `Int`, `Float`, `Bool`, and `String` methods.

```cpp
#include "cppflag.hpp"

cli::FlagSet fs("my_app", "An example application");
fs.Int("port", 8080, "port to listen on");
fs.Bool("debug", false, "enable debug logging");
fs.Float("ratio", 1.5, "a float value");
fs.String("mode", "fast", "running mode");
```

### 2. Parsing Arguments

Call the `Parse` method with `argc` and `argv` to parse the command-line arguments.

```cpp
cli::ParseResult pr = fs.Parse(argc, argv);
if (!pr) {
    fs.PrintError(pr, std::cerr);
    std::cerr << "\n";
    fs.PrintUsage(std::cerr);
    return 2;
}
```

### 3. Accessing Flag Values

There are two ways to access the parsed values:

**Method 1: Use the `Flag*` pointer (most efficient)**

When you define a flag, the method returns a `Flag*` pointer. You can store this pointer and use its `As<T>()` method to get the value after parsing.

```cpp
auto portFlag = fs.Int("port", 8080, "port to listen on");
int64_t port = portFlag->As<int64_t>();
```

**Method 2: Use the `Get<T>` helper function (convenient)**

This helper function looks up the flag by name and returns its value. This is more convenient if you don't want to store the `Flag*` pointers.

```cpp
int64_t port = cli::Get<int64_t>(fs, "port");
bool debug = cli::Get<bool>(fs, "debug");
double ratio = cli::Get<double>(fs, "ratio");
std::string mode = cli::Get<std::string>(fs, "mode");
```

### 4. Handling Positional Arguments

Any arguments that are not flags or flag values are treated as positional arguments. You can access them using the `Positional()` method.

```cpp
for (const auto& p : fs.Positional()) {
    std::cout << "Positional argument: " << p << "\n";
}
```

### 5. Checking if a Flag Was Set

You can use the `IsSet` method to check if a flag was explicitly set by the user on the command line.

```cpp
if (fs.IsSet("port")) {
    std::cout << "The port was set by the user.\n";
}
```

## Full Example

A complete example can be found in `examples/full_demo.cpp`.

To compile and run the example:

```bash
g++ -std=c++17 full_demo.cpp -o full_demo_app
./full_demo_app --port=-9090 --debug --ratio=2.5 arg1 arg2
```

This will produce the following output:

```
=== Final Configuration ===
port  = -9090
debug = true
ratio = 2.5
mode  = fast
Which were set by user?
  port: user
  debug: user
  ratio: user
  mode: default
Positional arguments:
  - arg1
  - arg2
```