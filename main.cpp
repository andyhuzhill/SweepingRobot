/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *       Revision:  none
 *       Compiler:  g++ 6.1.1
 *       Compile Flags: -Wall --std=c++11 -lcurl
 *
 * =====================================================================================
 */

#include <iostream>
#include <curl/curl.h>
#include <memory>
#include <regex>

using namespace std;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    string src = string(static_cast<char *>(buffer));
    regex match_map("level=\\d+&x=\\d+&y=\\d+&map=\\d+", regex_constants::icase);

    auto smatch = sregex_iterator(src.begin(), src.end(), match_map);
    auto match_end = sregex_iterator();

    if (smatch != match_end) {
        string match_string = (*smatch).str();

        regex level_match("level=\\d+");
        regex row_match("x=\\d+");
        regex col_match("y=\\d+");
        regex map_match("map=\\d+");

        auto level_match_iter = sregex_iterator(match_string.begin(), match_string.end(), level_match);
        string level_string = (*level_match_iter).str();

        level_string.erase(0, 6);
        std::cout << "level = " << stol(level_string) << std::endl;

        auto col_match_iter = sregex_iterator(match_string.begin(), match_string.end(), col_match);
        string col_string = (*col_match_iter).str();

        col_string.erase(0, 2);
        int col = stol(col_string);
        std::cout << "col = " << col << std::endl;

        auto row_match_iter = sregex_iterator(match_string.begin(), match_string.end(), row_match);
        string row_string = (*row_match_iter).str();

        row_string.erase(0, 2);
        int row = stol(row_string);
        std::cout << "row = " << row << std::endl;

        auto map_match_iter = sregex_iterator(match_string.begin(), match_string.end(), map_match);
        string map_string = (*map_match_iter).str();

        map_string.erase(0, 4);
        std::cout << "map = " << (map_string) << std::endl;

        int idx = 0;
        for (int r = 0 ; r < row ; ++r ) {
            for ( int c = 0 ; c < col ; ++c ) {
                if (map_string.at(idx) == '1') {
                    cout << "#";
                } else {
                    cout << "*";
                }
                ++idx;
            }
            cout << endl;
        }
        cout << endl;
    }

    return size * nmemb;
}

int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);

    auto handle = curl_easy_init();

    curl_easy_setopt(handle, CURLOPT_URL, "http://www.qlcoder.com/train/autocr");
    curl_easy_setopt(handle, CURLOPT_COOKIE, "laravel_session=eyJpdiI6IkhFckdFZkZOam9wOW9IRmxNSXBaMWc9PSIsInZhbHVlIjoiZFRuT1A2d0V5bjNpMGZVTXYyRGJYZlhQQUhLbjRlNVZ1Q2U1MDhLTkdtRStUZHR3UmdPY3NRdWNFYUNqNEJxRFhGdmM1aWtQc2FMd1g1Wk1NQXNvUUE9PSIsIm1hYyI6ImM1NWJjY2NjNTZiNTM1NzU0MTg1MWEyNzliMmVkZmRhOTU3NjI3MmUxYjg0OTkwOWI0MjhmN2ZjMjM0NTFkYjIifQ%3D%3D;");
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);

    CURLcode success = curl_easy_perform(handle);
    if (success != CURLE_OK) {
        cout << "Can't perform url operations!" << endl;
    }

    curl_global_cleanup();
    return 0;
}
