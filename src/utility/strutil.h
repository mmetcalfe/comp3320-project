#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

namespace utility {
namespace strutil {
    inline std::string getFileString(const std::string fileName) {
        std::ifstream ifs(fileName, std::ifstream::binary);

        if (!ifs.good()) {
            std::stringstream errMsg;
            errMsg << __func__ << ": Could not open file '" << fileName << "'.";
            throw std::invalid_argument(errMsg.str());
        }

        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()    ) );

        return content;
    }

    inline bool checkFirstBytes(const std::string fileName, const std::string cmpStr) {
        std::ifstream ifs(fileName, std::ifstream::binary);

        if (!ifs.good()) {
            std::stringstream errMsg;
            errMsg << __func__ << ": Could not open file '" << fileName << "'.";
            throw std::invalid_argument(errMsg.str());
        }

        std::istreambuf_iterator<char> fileIterator(ifs);
        for (auto c : cmpStr) {
            if (c != *fileIterator++)
                return false;
        }

        return true;
    }
}
}

