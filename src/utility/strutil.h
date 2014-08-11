#pragma once

#include <string>
#include <sstream>
#include <fstream>

namespace utility {
namespace strutil {
    inline std::string getFileString(const std::string fileName) {
        std::ifstream ifs(fileName);

        if (!ifs.good()) {
            std::stringstream errMsg;
            errMsg << __func__ << ": Could not open file '" << fileName << "'.";
            throw std::invalid_argument(errMsg.str().c_str());
        }

        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()    ) );

        return content;
    }
}
}

