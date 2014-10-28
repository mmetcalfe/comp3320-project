#pragma once
#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Profiler {
public:
    struct ProfilerNode {
        std::map<std::string, std::shared_ptr<ProfilerNode>> childrenMap;
        std::deque<std::chrono::microseconds> durations;
//        std::map<std::string, std::deque<std::chrono::microseconds>> durationMap;

        inline double average() {
            double childSum = 0;
            for (const auto& pair : childrenMap) {
                childSum += pair.second->average();
            }

            if (durations.empty())
                return childSum;

            double avg = 0;

            for (auto d : durations) {
                avg += (double)d.count();
            }

            double ms = avg / (durations.size() * 1000.0); // milliseconds

            return childSum + ms;
        }

        inline void print(int nesting = 0) {
            int maxlen = 0;
            double total = 0;

            std::vector<std::pair<std::string, std::shared_ptr<ProfilerNode>>> data;
            for (const auto& pair : childrenMap) {
                auto node = pair.second;

                maxlen = std::max((int)pair.first.length(), maxlen);
                double avg = node->average();// average(pair.first);
                total += avg;
                data.emplace_back(pair);
            }
            sort(data.begin(), data.end(), [](std::pair<std::string, std::shared_ptr<ProfilerNode>>& p1, std::pair<std::string, std::shared_ptr<ProfilerNode>>& p2) {
                return p1.second->average() > p2.second->average();
            });

            for (const auto& pair : data) {
                double avg = pair.second->average();
                std::cout
                        << std::setw(nesting * 4)
                        << "| "
                        << std::setw(maxlen)
                        << pair.first << ": "
                        << std::fixed
                        << std::setw(10)
                        << std::setprecision(4)
//                    << average(pair.first) << " ms"
                        << avg << " ms, "
                        << std::setw(5)
                        << std::setprecision(2)
                        << (100 * avg / total)
                        << "%"
                        << std::endl;

                pair.second->print(nesting + 1);
            }
        }
    };

    std::deque<std::shared_ptr<ProfilerNode>> nodeStack;
    std::shared_ptr<ProfilerNode> currentNode;

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point lastPrint;
    unsigned sampleLimit;
    bool glFinishEnabled;
    bool disabled = false;

    inline Profiler(unsigned sampleLimit = 100) {
        this->sampleLimit = sampleLimit;
        lastPrint = std::chrono::system_clock::now();
        glFinishEnabled = true;
        disabled = false;

        currentNode = std::make_shared<ProfilerNode>();
        nodeStack.push_back(currentNode);

        reset();
    };

    inline void reset() {
        start = std::chrono::system_clock::now();
    }

    inline void disable() {
        disabled = true;
    }

    inline void enable() {
        disabled = false;
    }

    inline void push(std::string label) {
        std::shared_ptr<ProfilerNode> childNode;
        if(currentNode->childrenMap.find(label) == currentNode->childrenMap.end()) {
            childNode = std::make_shared<ProfilerNode>();
            currentNode->childrenMap[label] = childNode;
        } else {
            childNode = currentNode->childrenMap[label];
        }

        nodeStack.push_back(childNode);
        currentNode = childNode;
    }

    inline void pop() {
        nodeStack.pop_back();
        currentNode = nodeStack.back();
    }

    inline void split(std::string label) {
        if (disabled)
            return;

        // Wait until the effects of all previously called GL commands are complete.
        if (glFinishEnabled)
            glFinish();

        auto end = std::chrono::system_clock::now();

        if(currentNode->childrenMap.find(label) == currentNode->childrenMap.end()) {
            currentNode->childrenMap[label] = std::make_shared<ProfilerNode>();
        }

        currentNode->childrenMap[label]->durations.push_front(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
        start = end;

        if (currentNode->childrenMap[label]->durations.size() > sampleLimit)
            currentNode->childrenMap[label]->durations.pop_back();
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

    inline void print() {
        std::cout << "Times:"
                << " (glFinishEnabled: " << std::boolalpha << glFinishEnabled << " (T to toggle))"
                << std::endl;

        currentNode->print();
    }

    inline void printEvery(double seconds) {
        auto current = std::chrono::system_clock::now();
        double micro = std::chrono::duration_cast<std::chrono::microseconds>(current - lastPrint).count();

        if (micro / 1000000.0 > seconds) {
            print();
            lastPrint = current;
        }
    }
};
