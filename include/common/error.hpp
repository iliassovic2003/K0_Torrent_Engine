#pragma once

#include <stdexcept>
#include <string>

class TrackerError : public std::runtime_error {
public:
    explicit TrackerError(const std::string& msg) : std::runtime_error(msg) {}
};

class NetworkError : public std::runtime_error {
public:
    explicit NetworkError(const std::string& msg) : std::runtime_error(msg) {}
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};