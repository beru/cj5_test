#include <stdio.h>
#include <vector>
#include <array>
#include <string_view>

#define CJ5_IMPLEMENT
#include "cj5.h"

void test_cj5(const char* filePath)
{
    FILE* f = fopen(filePath, "rb");
    if (!f) {
        return;
    }
    fseek(f, 0L, SEEK_END);
    int sz = (int)ftell(f);
    fseek(f, 0L, SEEK_SET);

    std::vector<char> buff(sz);
    char* js = &buff[0];
    fread(js, 1, sz, f);
    fclose(f);

    cj5_result res = cj5_parse(js, sz, nullptr, 0);
    if (res.error != CJ5_ERROR_NONE) {
        return;
    }
    std::vector<cj5_token> tokens(res.num_tokens);
    int hoge = 0;

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
