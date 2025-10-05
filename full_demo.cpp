#include "cppflag.hpp" 
 #include <iostream> 
 #include <iomanip> 
 
 int main(int argc, char** argv) { 
     cli::FlagSet fs("full_demo", "A full demo for FlagSet") ; 
 
     fs.Int("port", 8080, "port to listen on" ); 
     fs.Bool("debug", false, "enable debug logging" ); 
     fs.Float("ratio", 1.0f, "ratio for calculation" ); 
     fs.String("mode", "fast", "running mode" ); 
 
     cli::ParseResult pr = fs.Parse (argc, argv); 
     if  (!pr) { 
         fs.PrintError (pr, std::cerr); 
         std::cerr << "\n" ; 
         fs.PrintUsage (std::cerr); 
         return 2 ; 
     } 
 
     auto port  = cli::Get<int64_t>(fs, "port"); 
     auto dbg   = cli::Get<bool>(fs, "debug"); 
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