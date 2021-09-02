#pragma once

#include <cstdint>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

class TokenServiceResponseData {
public:
    time_t expireTime;
    std::string type;
    std::string token;
};

class TokenServiceResponse {
public:
    std::string result;
    TokenServiceResponseData data;
};

void from_json(const json &j, TokenServiceResponseData &t);
void ts_data_to_json(json &j, const TokenServiceResponseData &t);