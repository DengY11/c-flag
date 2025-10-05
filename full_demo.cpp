#include "cppflag.hpp" 
 #include <iostream> 
 #include <iomanip> 
 
 int main(int argc, char** argv) { 
     cli::FlagSet fs("full_demo", "A full demo for FlagSet") ; 
 
     auto portFlag = fs.Int("port", 8080, "port to listen on", 'p'); 
     auto debugFlag = fs.Bool("debug", false, "enable debug logging", 'd'); 
     auto ratioFlag = fs.Float("ratio", 1.0f, "ratio for calculation", 'r'); 
     auto modeFlag = fs.String("mode", "fast", "running mode", 'm'); 
 
     cli::ParseResult pr = fs.Parse (argc, argv); 
     if (pr.kind == cli::ParseErrorKind::HelpRequested) {
         fs.PrintUsage(std::cout);
         return 0;
     }
     if  (!pr) { 
         fs.PrintError (pr, std::cerr); 
         std::cerr << "\n" ; 
         fs.PrintUsage (std::cerr); 
         return 2 ; 
     } 
 
     // Accessing values using the Flag* pointers returned during definition. 
     auto port  = portFlag->As<int64_t>(); 
     auto dbg   = debugFlag->As<bool>(); 
     auto ratio = ratioFlag->As<double>(); 
     auto mode  = modeFlag->As<std::string>(); 
 
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