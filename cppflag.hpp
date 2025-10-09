#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace cli {

enum class ParseErrorKind {
  None,
  HelpRequested,
  UnknownFlag,
  MissingValue,
  InvalidValue,
};

struct ParseResult {
  ParseErrorKind kind = ParseErrorKind::None;
  std::string flag;
  std::string message;
  bool ok() const { return kind == ParseErrorKind::None; }
  explicit operator bool() const { return ok(); }
};

class IValue {
public:
  virtual ~IValue() = default;
  /* Set parses the string argument and sets the value.
     It returns true on success and false on failure, setting err to an error
     message. */
  virtual bool Set(std::string_view text, std::string &err) = 0;
  /* TypeName returns the type name of the value. */
  virtual std::string TypeName() const = 0;
  /* ToString returns the string representation of the value. */
  virtual std::string ToString() const = 0;
  /* clone returns a copy of the value object. */
  virtual IValue *clone() const = 0;
};

template <typename T> class ValueAdapter : public IValue {
public:
  using Tp = std::remove_cv_t<std::decay_t<T>>;
  explicit ValueAdapter(T val) : value_(val) {
    if constexpr (std::is_same<Tp, int64_t>::value) {
      type_name_ = "int";
    } else if constexpr (std::is_same<Tp, float>::value ||
                         std::is_same<Tp, double>::value) {
      type_name_ = "float";
    } else if constexpr (std::is_same<Tp, bool>::value) {
      type_name_ = "bool";
    } else if constexpr (std::is_same<Tp, std::string>::value) {
      type_name_ = "string";
    }
  };
  bool Set(std::string_view text, std::string &err) override {
    if constexpr (std::is_same<Tp, int64_t>::value) {
      try {
        value_ = std::stoll(std::string(text));
      } catch (const std::invalid_argument &ia) {
        err = "not an integer";
        return false;
      } catch (const std::out_of_range &oor) {
        err = "out of range for int64_t";
        return false;
      }
    } else if constexpr (std::is_same<Tp, float>::value ||
                         std::is_same<Tp, double>::value) {
      try {
        value_ = std::stod(std::string(text));
      } catch (const std::invalid_argument &ia) {
        err = "not a float";
        return false;
      } catch (const std::out_of_range &oor) {
        err = "out of range for float";
        return false;
      }
    } else if constexpr (std::is_same<Tp, bool>::value) {
      // supports true/false, 1/0, yes/no
      std::string lower_text;
      for (char c : text) {
        lower_text += std::tolower(c);
      }

      if (lower_text == "true" || lower_text == "1" || lower_text == "yes" ||
          lower_text == "on") {
        value_ = true;
      } else if (lower_text == "false" || lower_text == "0" ||
                 lower_text == "no" || lower_text == "off") {
        value_ = false;
      } else {
        err = "invalid boolean value, accepts true/false, 1/0, yes/no, on/off";
        return false;
      }

    } else if constexpr (std::is_same<Tp, std::string>::value) {
      value_ = text;
    } else {
      err = "set unknown type";
      return false;
    }
    return true;
  }

  std::string ToString() const override {
    if constexpr (std::is_same<T, bool>::value) {
      return value_ ? "true" : "false";
    } else if constexpr (std::is_same<T, std::string>::value) {
      return value_;
    } else {
      return std::to_string(value_);
    }
  }

  std::string TypeName() const override { return type_name_; }
  const T &Get() const { return value_; }
  virtual IValue *clone() const override { return new ValueAdapter<T>(value_); }

private:
  T value_;
  std::string type_name_;
};

struct Flag {
  std::string name;
  char short_name = 0;
  std::string usage;
  std::unique_ptr<IValue> value;
  std::unique_ptr<IValue> default_value;
  bool set = false;

  /* As returns the value of the flag as type T.
     It throws std::bad_cast if the type does not match. */
  template <typename T> const T &As() const {
    auto va = dynamic_cast<const ValueAdapter<T> *>(value.get());
    if (!va) {
      throw std::bad_cast();
    }
    return va->Get();
  }
};

class FlagSet {
public:
  /* FlagSet creates a new, empty flag set with the specified name and
   * description. */
  explicit FlagSet(std::string name, std::string desc = {});
  /* Int defines a int64_t flag with specified name, default value, and usage
   * string. */
  Flag *Int(std::string_view name, int64_t defaultVal, std::string_view usage,
            char short_name = 0);
  /* Float defines a double flag with specified name, default value, and usage
   * string. */
  Flag *Float(std::string_view name, double defaultVal, std::string_view usage,
              char short_name = 0);
  /* Bool defines a bool flag with specified name, default value, and usage
   * string. */
  Flag *Bool(std::string_view name, bool defaultVal, std::string_view usage,
             char short_name = 0);
  /* String defines a string flag with specified name, default value, and usage
   * string. */
  Flag *String(std::string_view name, std::string_view defaultVal,
               std::string_view usage, char short_name = 0);

  /* Parse parses flag definitions from the argument list, which should not
     include the command name. It returns a ParseResult indicating success or
     failure. */
  ParseResult Parse(int argc, char **argv);
  /* Lookup returns the Flag structure for a flag, or nullptr if not found. */
  const Flag *Lookup(std::string_view name) const;
  /* IsSet reports whether the flag was set by the user. */
  bool IsSet(std::string_view name) const;

  // Usage
  /* PrintUsage prints a usage message to the given output stream. */
  void PrintUsage(std::ostream &os) const;
  /* PrintError prints an error message to the given output stream. */
  void PrintError(const ParseResult &pr, std::ostream &os) const;
  /* Positional returns the non-flag arguments. */
  std::vector<std::string> Positional() const { return positional_; }

private:
  std::string name_;
  std::string desc_;
  std::vector<std::unique_ptr<Flag>> flags_;
  std::unordered_map<std::string_view, Flag *> index_;
  std::unordered_map<char, Flag *> short_index_;
  std::vector<std::string> positional_;
  template <typename T>
  Flag *AddFlag(std::string_view name, char short_name, T defaultVal,
                std::string_view usage);
};

FlagSet::FlagSet(std::string name, std::string desc) {
  name_ = name;
  desc_ = desc;
  Bool("help", false, "show this help message", 'h');
}

template <typename T>
Flag *FlagSet::AddFlag(std::string_view name, char short_name, T defaultVal,
                       std::string_view usage) {
  auto ptr = std::make_unique<Flag>();
  ptr->name = name;
  ptr->usage = usage;
  ptr->short_name = short_name;
  auto v_ptr = std::make_unique<ValueAdapter<T>>(defaultVal);
  ptr->default_value = std::unique_ptr<IValue>(v_ptr->clone());
  ptr->value = std::move(v_ptr);
  ptr->set = false;
  this->flags_.emplace_back(std::move(ptr));
  Flag *flag_ptr = this->flags_.back().get();
  this->index_[name] = flag_ptr;
  if (short_name != 0) {
    this->short_index_[short_name] = flag_ptr;
  }
  return flag_ptr;
}

Flag *FlagSet::Int(std::string_view name, int64_t defaultVal,
                   std::string_view usage, char short_name) {
  return AddFlag<int64_t>(name, short_name, defaultVal, usage);
}

Flag *FlagSet::Float(std::string_view name, double defaultVal,
                     std::string_view usage, char short_name) {
  return AddFlag<double>(name, short_name, defaultVal, usage);
}

Flag *FlagSet::Bool(std::string_view name, bool defaultVal,
                    std::string_view usage, char short_name) {
  return AddFlag<bool>(name, short_name, defaultVal, usage);
}

Flag *FlagSet::String(std::string_view name, std::string_view defaultVal,
                      std::string_view usage, char short_name) {
  return AddFlag<std::string>(name, short_name, std::string(defaultVal), usage);
}

/* Parse parses flag definitions from the argument list, which should not
   include the command name. It returns a ParseResult indicating success or
   failure. */
ParseResult FlagSet::Parse(int argc, char **argv) {
  positional_.clear();
  bool no_more_flags = false;

  for (const auto &flag : flags_) {
    flag->value.reset(flag->default_value->clone());
    flag->set = false;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h" || arg == "-help") {
      return {ParseErrorKind::HelpRequested, "", ""};
    }

    if (no_more_flags) {
      positional_.push_back(arg);
      continue;
    }

    if (arg == "--") {
      no_more_flags = true;
      continue;
    }

    // handle positional arguments
    if (arg[0] != '-') {
      positional_.push_back(arg);
      continue;
    }

    // handle long options --flag=value or --flag value
    if (arg.size() > 2 && arg[1] == '-') {
      std::string flag_name;
      std::string value;

      size_t equal_pos = arg.find('=');
      if (equal_pos != std::string::npos) {
        // --flag=value format
        flag_name = arg.substr(2, equal_pos - 2);
        value = arg.substr(equal_pos + 1);
      } else {
        // --flag value format
        flag_name = arg.substr(2);
        auto it = index_.find(flag_name);
        if (it != index_.end()) {
          auto va = dynamic_cast<ValueAdapter<bool> *>(it->second->value.get());
          if (va) {
            value = "true";
          } else if (i + 1 < argc && argv[i + 1][0] != '-') {
            value = argv[++i];
          } else {
            return ParseResult{ParseErrorKind::MissingValue, flag_name,
                               "flag '" + flag_name + "' needs a value"};
          }
        }
      }

      auto it = index_.find(flag_name);
      if (it == index_.end()) {
        return ParseResult{ParseErrorKind::UnknownFlag, flag_name,
                           "unknown flag: " + flag_name};
      }

      Flag *flag = it->second;
      std::string error;
      if (!flag->value->Set(value, error)) {
        return ParseResult{ParseErrorKind::InvalidValue, flag_name,
                           "invalid value for flag '" + flag_name +
                               "': " + error};
      }
      flag->set = true;
    }
    // handle short options -f value or -fvalue
    else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
      char flag_char = arg[1];
      auto it = short_index_.find(flag_char);
      if (it == short_index_.end()) {
        return ParseResult{ParseErrorKind::UnknownFlag,
                           std::string(1, flag_char),
                           "unknown flag: -" + std::string(1, flag_char)};
      }
      Flag *flag = it->second;
      std::string value;

      if (arg.size() > 2) {
        // -fvalue format
        value = arg.substr(2);
      } else {
        // -f format, value might be next arg, or implicit for bool
        auto va = dynamic_cast<ValueAdapter<bool> *>(flag->value.get());
        if (va) {
          value = "true";
        } else if (i + 1 < argc) {
          value = argv[++i];
        } else {
          return ParseResult{ParseErrorKind::MissingValue, flag->name,
                             "flag '-" + std::string(1, flag_char) +
                                 "' needs a value"};
        }
      }

      std::string error;
      if (!flag->value->Set(value, error)) {
        return ParseResult{ParseErrorKind::InvalidValue, flag->name,
                           "invalid value for flag '-" +
                               std::string(1, flag_char) + "': " + error};
      }
      flag->set = true;
    }
  }

  return ParseResult{};
}

const Flag *FlagSet::Lookup(std::string_view name) const {
  auto it = index_.find(name);
  if (it != index_.end()) {
    return it->second;
  }
  return nullptr;
}

bool FlagSet::IsSet(std::string_view name) const {
  auto flag = Lookup(name);
  return flag && flag->set;
}

void FlagSet::PrintUsage(std::ostream &os) const {
  os << "Usage: " << name_;
  if (!flags_.empty()) {
    os << " [flags]";
  }
  os << "\n";

  if (!desc_.empty()) {
    os << desc_ << "\n";
  }

  if (!flags_.empty()) {
    os << "\nFlags:\n";
    for (const auto &flag : flags_) {
      os << "  ";
      if (flag->short_name != 0) {
        os << "-" << flag->short_name << ", ";
      }
      os << "--" << flag->name;
      os << "\t" << flag->usage
         << " (default: " << flag->default_value->ToString() << ")\n";
    }
  }
}

void FlagSet::PrintError(const ParseResult &pr, std::ostream &os) const {
  os << "error: " << pr.message;
}

/* Get returns the value of the flag with the given name from the flag set.
   It returns a zero value if the flag is not found. */
template <typename T> T Get(const FlagSet &fs, std::string_view name) {
  auto f = fs.Lookup(name);
  if (f) {
    return f->As<T>();
  }
  return T{};
}

} // namespace cli
