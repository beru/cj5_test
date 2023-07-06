#include <stdio.h>
#include <vector>
#include <array>
#include <string_view>
#include <charconv>

#define CJ5_IMPLEMENT
#define CJ5_TOKEN_HELPERS 0
#include "cj5.h"

inline
std::string_view tosv(cj5_token* token, const char* js)
{
    return std::string_view(js + token->start, token->end - token->start);
}

template <typename FuncT>
bool readObject(cj5_token*& ptoken, const char* js, FuncT&& fn)
{
    if (ptoken->type != CJ5_TOKEN_OBJECT) return false;
    const int sz = ptoken->size;
    ++ptoken;
    for (int i=0; i<sz; ++i) {
        std::string_view propName = tosv(ptoken, js);
        ++ptoken;
        if (!fn(ptoken, propName)) return false;
    }
    return true;
}

template <typename FuncT>
bool readArray(cj5_token*& ptoken, const char* js, FuncT&& fn)
{
    if (ptoken->type != CJ5_TOKEN_ARRAY) return false;
    const int sz = ptoken->size;
    ++ptoken;
    for (int i=0; i<sz; ++i) {
        if (!fn(ptoken)) return false;
    }
    return true;
}

bool readString(cj5_token*& ptoken, const char* js, std::string_view& sv)
{
    if (ptoken->type != CJ5_TOKEN_STRING) return false;
    sv = tosv(ptoken, js);
    ++ptoken;
    return true;
}

template <typename T>
bool readNumber(cj5_token*& ptoken, const char* js, T& sv)
{
    if (ptoken->type != CJ5_TOKEN_NUMBER) return false;
    if (ptoken->num_type == CJ5_TOKEN_NUMBER_HEX) {
        sv = std::strtol(js + ptoken->start, nullptr, 16);
    }else {
        std::from_chars_result res = std::from_chars(js + ptoken->start, js + ptoken->end, sv);
        //if (res.ec == )
    }
    ++ptoken;
    return true;
}


void test_cj5(const char* filePath)
{
    FILE* f = fopen(filePath, "rb");
    if (!f) {
        printf("failed to open file %s\n", filePath);
        return;
    }
    fseek(f, 0L, SEEK_END);
    int sz = (int)ftell(f);
    fseek(f, 0L, SEEK_SET);

    std::vector<char> buff(sz);
    char* js = &buff[0];
    fread(js, 1, sz, f);
    fclose(f);

    cj5_result res;
    res = cj5_parse(js, sz, nullptr, 0);
    if (res.error != CJ5_ERROR_NONE) {
        printf("cj5_parse failed : %d.\n", res.error);
        return;
    }
    std::vector<cj5_token> tokens(res.num_tokens);
    cj5_token* ptoken = &tokens[0];
    res = cj5_parse(js, sz, ptoken, res.num_tokens);
    if (res.error != CJ5_ERROR_NONE) {
        printf("cj5_parse failed : %d.\n", res.error);
        return;
    }

    struct Sample {
        std::string_view unquoted;
        std::string_view singleQuotes;
        std::string_view lineBreaks;
        int hexadecimal;
        double leadingDecimalPoint;
        double andTrailing;
        double positiveSign;
        std::string_view trailingComma;
        std::array<std::string_view, 3> andIn;
        std::string_view backwardsCompatible;
    } s;

    if (!readObject(ptoken, js, [&](cj5_token*& ptoken, std::string_view name){
        if (name == "unquoted") {
            if (!readString(ptoken, js, s.unquoted)) return false;
        }else if (name == "singleQuotes") {
            if (!readString(ptoken, js, s.singleQuotes)) return false;
        }else if (name == "lineBreaks") {
            if (!readString(ptoken, js, s.lineBreaks)) return false;
        }else if (name == "hexadecimal") {
            if (!readNumber(ptoken, js, s.hexadecimal)) return false;
        }else if (name == "leadingDecimalPoint") {
            if (!readNumber(ptoken, js, s.leadingDecimalPoint)) return false;
        }else if (name == "andTrailing") {
            if (!readNumber(ptoken, js, s.andTrailing)) return false;
        }else if (name == "positiveSign") {
            if (!readNumber(ptoken, js, s.positiveSign)) return false;
        }else if (name == "trailingComma") {
            if (!readString(ptoken, js, s.trailingComma)) return false;
        }else if (name == "andIn") {
            int idx = 0;
            if (!readArray(ptoken, js, [&](cj5_token*& ptoken){
                if (idx >= s.andIn.size()) return false;
                if (!readString(ptoken, js, s.andIn[idx])) return false;
                ++idx;
                return true;
            })) {
                return false;
            }
        }else if (name == "backwardsCompatible") {
            if (!readString(ptoken, js, s.backwardsCompatible)) return false;
        }else {
            return false;
        }
        return true;
    })) {
        printf("failed to read json5.\n");
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("specify filename.\n");
        return -1;
    }
    test_cj5(argv[1]);
    return 0;
}
