#include <fstream>
#include <string.h>
#include "inipp.h"

namespace IniLoader {
    inipp::Ini<char> ini;
    std::string fileUrl;

    // loading .ini file
    void loadINIFile( std::string fileUrlParam ) {
        fileUrl = fileUrlParam;
        std::ifstream is( fileUrlParam );
        ini.parse(is);
        ini.strip_trailing_comments();
        ini.default_section(ini.sections["default"]);
        ini.interpolate();
    }

    void getValue( std::string section, std::string variableName, auto& destinationVar ) {
        inipp::get_value(ini.sections[section], variableName, destinationVar);
    }

};