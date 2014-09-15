#pragma once
#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Profiler {
public:
    inline Profiler(unsigned sampleLimit = 100) {
        this->sampleLimit = sampleLimit;
        lastPrint = std::chrono::system_clock::now();
        reset();
    };

    inline void reset() {
        start = std::chrono::system_clock::now();
    }

    inline void split(std::string label) {
        // Wait until the effects of all previously called GL commands are complete.
        glFinish();

        auto end = std::chrono::system_clock::now();
        durationMap[label].push_front(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
        start = end;

        if (durationMap[label].size() > sampleLimit)
            durationMap[label].pop_back();
    }
    // Allow any sequence of stringstreamable arguments to be passed to split.
    template<typename T, typename... Args>
    inline void split(std::string s, T t, Args... args) {
        std::stringstream ss;
        ss << s << t;

        split(ss.str(), args...);
    }
    template<typename T, typename... Args>
    inline void split(T t, Args... args) {
        std::stringstream ss;
        ss << t;

        split(ss.str(), args...);
    }

    inline double average(std::string label) {
        double avg = 0;

        for (auto d : durationMap[label]) {
            avg += (double)d.count();
        }

//        return avg / (durationMap[label].size() * 1000000.0); // seconds
        return avg / (durationMap[label].size() * 1000.0); // milliseconds
//        return avg / durationMap[label].size(); // microseconds
    }

    inline void print() {
        // TODO: Sort the output by order added / duration.
        std::cout << "Times:" << std::endl;
        for (const auto& pair : durationMap) {
            std::cout << "  "
                    <<  std::setw(15)
                    << pair.first << ": "
                    << std::fixed
                    << std::setw(10)
                    << std::setprecision(4)
                    << average(pair.first)
                    << " ms" << std::endl;
        }
    }

    inline void printEvery(double seconds) {
        auto current = std::chrono::system_clock::now();
        double micro = std::chrono::duration_cast<std::chrono::microseconds>(current - lastPrint).count();

        if (micro / 1000000.0 > seconds) {
            print();
            lastPrint = current;
        }
    }

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point lastPrint;
    unsigned sampleLimit;
    std::map<std::string, std::deque<std::chrono::microseconds>> durationMap;
};
