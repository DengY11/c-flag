#include "cppflag.hpp" 
 #include <iostream> 
 #include <iomanip> 
 
 int main(int argc, char** argv) { 
     cli::FlagSet fs("full_demo", "A full demo for FlagSet") ; 
 
     auto portFlag = fs.Int("port", 8080, "port to listen on" ); 
     auto debugFlag = fs.Bool("debug", false, "enable debug logging" ); 
     fs.Float("ratio", 1.0f, "ratio for calculation" ); 
     fs.String("mode", "fast", "running mode" ); 
 
     cli::ParseResult pr = fs.Parse (argc, argv); 
     if  (!pr) { 
         fs.PrintError (pr, std::cerr); 
         std::cerr << "\n" ; 
         fs.PrintUsage (std::cerr); 
         return 2 ; 
     } 
 
     // Accessing values: 
     // Method 1: Use the Flag* returned from the definition (most efficient). 
     auto port  = portFlag->As<int64_t>(); 
     auto dbg   = debugFlag->As<bool>(); 
 
     // Method 2: Use the Get<T> helper function (convenient). 
     auto ratio = cli::Get<double>(fs, "ratio"); 
     auto mode  = cli::Get<std::string>(fs, "mode"); 
 
     std::cout << "=== Final Configuration ===\n" ; 
     std::cout << "port  = " << port << "\n" ; 
     std::cout << "debug = " << std::boolalpha << dbg << "\n" ; 
     std::cout << "ratio = " << ratio << "\n" ; 
     std::cout << "mode  = " << mode << "\n" ; 
     std::cout << "Which were set by user?\n" ; 
     for (auto name : {"port","debug","ratio","mode" }) { 
         std::cout << "  " << name << ": " << (fs.IsSet(name) ? "user" : "default") << "\n" ; 
     } 
 
     if (!fs.Positional().empty ()) { 
         std::cout << "Positional arguments:\n" ; 
         for (auto& p : fs.Positional ()) { 
             std::cout << "  - " << p << "\n" ; 
         } 
     } else  { 
         std::cout << "No positional arguments\n" ; 
     } 
     return 0 ; 
 }