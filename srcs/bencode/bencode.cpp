#include "../../include/bencode/bencode.hpp"

static BencodeValue parse(const std::string& data, size_t& pos) {
    if (pos >= data.size())
        throw std::runtime_error("unexpected end of data");

    char c = data[pos];

    // ── integer ──────────────────────────────────────────────────────────────
    if (c == 'i') {
        pos++;
        size_t start = pos;
        while (pos < data.size() && data[pos] != 'e')
            pos++;
        if (pos >= data.size())
            throw std::runtime_error("unterminated integer");
        std::string num = data.substr(start, pos - start);
        if (num.empty())
            throw std::runtime_error("empty integer");
        for (size_t i = (num[0] == '-' ? 1 : 0); i < num.size(); i++)
            if (!std::isdigit((unsigned char)num[i]))
                throw std::runtime_error("invalid integer: " + num);
        int64_t val = std::stoll(num);
        pos++; // skip 'e'
        return BencodeValue{ val };
    }

    // ── string ───────────────────────────────────────────────────────────────
    if (std::isdigit((unsigned char)c)) {
        size_t start = pos;
        while (pos < data.size() && data[pos] != ':')
            pos++;
        if (pos >= data.size())
            throw std::runtime_error("malformed string: no colon");
        size_t len = std::stoull(data.substr(start, pos - start));
        pos++; // skip ':'
        if (pos + len > data.size())
            throw std::runtime_error("string length out of bounds");
        std::string str = data.substr(pos, len);
        pos += len;
        return BencodeValue{ str };
    }

    // ── list ─────────────────────────────────────────────────────────────────
    if (c == 'l') {
        pos++;
        BencodeValue::List list;
        while (pos < data.size() && data[pos] != 'e')
            list.push_back(parse(data, pos));
        if (pos >= data.size())
            throw std::runtime_error("unterminated list");
        pos++;
        return BencodeValue{ list };
    }

    // ── dict ─────────────────────────────────────────────────────────────────
    if (c == 'd') {
        pos++;
        BencodeValue::Dict dict;
        while (pos < data.size() && data[pos] != 'e') {
            auto key = parse(data, pos);
            if (!key.is_string())
                throw std::runtime_error("dict key must be a string");
            auto val = parse(data, pos);
            dict[key.as_string()] = val;
        }
        if (pos >= data.size())
            throw std::runtime_error("unterminated dict");
        pos++;
        return BencodeValue{ dict };
    }

    throw std::runtime_error(std::string("unknown bencode type: ") + c);
}

BencodeValue decode(const std::string& data, size_t& pos) {
    return parse(data, pos);
}