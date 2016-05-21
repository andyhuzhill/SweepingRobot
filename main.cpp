/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  robot
 *
 *        Version:  1.0
 *       Revision:  none
 *       Compiler:  g++ 4.9.2
 *       Compile Flags: -std=c++11 -Wall -Werror -lcurl
 *
 * =====================================================================================
 */

#include <iostream>
#include <curl/curl.h>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#define DEBUG 0

using namespace std;

class Solution
{
public:
    Solution(const string &get, const string &post)
        : get_url(get),
          post_url(post)
    {
        handler = curl_easy_init();
    }

    void SetCookies(const string &cookie)
    {
        curl_easy_setopt(handler, CURLOPT_COOKIE, cookie.c_str());
    }

    void GetMap()
    {
        got_map = false;

        curl_easy_setopt(handler, CURLOPT_URL, get_url.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, Solution::GetMapWriteDat);

        CURLcode code = curl_easy_perform(handler);
        if (code != CURLE_OK) {
            cerr << "Can't perform Get operations!" << endl;
        }
    }

    void PostSolution()
    {
        if (m_path.empty()) {
            cerr << "Solution Not Found!" << endl;
            return ;
        }
        string post = post_url + "?x=" + to_string(start_posy + 1) + "&y=" + to_string(start_posx + 1) + "&path=" + m_path;

        curl_easy_setopt(handler, CURLOPT_URL, post.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, Solution::PostDataFunction);

        CURLcode code = curl_easy_perform(handler);
        if (code != CURLE_OK) {
            cerr << "Can't perform Post operation!" << endl;
        }
    }

    bool CleanRoom(const char *map, const int &row, const int &col, int posx, int posy, const string &path = "")
    {
        bool result = false;

        bool canTurnUp    = CanTurnUp(map, row, col, posx, posy);
        bool canTurnRight = CanTurnRight(map, row, col, posx, posy);
        bool canTurnDown  = CanTurnDown(map, row, col, posx, posy);
        bool canTurnLeft  = CanTurnLeft(map, row, col, posx, posy);

        int map_size = row * col;
        unique_ptr<char> data = unique_ptr<char>(new char[map_size]);
        memcpy(data.get(), map, map_size);
        if (*(data.get() + posy * col + posx) == 0) {
            *(data.get() + posy * col + posx) = 1;
        }

        unique_ptr<char> data1 = unique_ptr<char>(new char[map_size]);
        memcpy(data1.get(), map, map_size);
        if (*(data1.get() + posy * col + posx) == 0) {
            *(data1.get() + posy * col + posx) = 1;
        }

        unique_ptr<char> data2 = unique_ptr<char>(new char[map_size]);
        memcpy(data2.get(), map, map_size);
        if (*(data2.get() + posy * col + posx) == 0) {
            *(data2.get() + posy * col + posx) = 1;
        }

        unique_ptr<char> data3 = unique_ptr<char>(new char[map_size]);
        memcpy(data3.get(), map, map_size);
        if (*(data3.get() + posy * col + posx) == 0) {
            *(data3.get() + posy * col + posx) = 1;
        }

        unique_ptr<char> data4 = unique_ptr<char>(new char[map_size]);
        memcpy(data4.get(), map, map_size);
        if (*(data4.get() + posy * col + posx) == 0) {
            *(data4.get() + posy * col + posx) = 1;
        }

#if 0
        PrintMap(data.get(), row, col);
#endif
        int d_posx = posx;
        int d_posy = posy;
        if (canTurnUp) {
            TurnUp(data1.get(), row, col, d_posx, d_posy);
            if (Prune(data1.get(), row, col) && CleanRoom(data1.get(), row, col, d_posx, d_posy, path + "u")) {
                return true;
            }
        }

        d_posx = posx;
        d_posy = posy;
        if (canTurnRight) {
            TurnRight(data2.get(), row, col, d_posx, d_posy);
            if (Prune(data2.get(), row, col) && CleanRoom(data2.get(), row, col, d_posx, d_posy, path + "r")) {
                return true;
            }
        }

        d_posx = posx;
        d_posy = posy;
        if (canTurnDown) {
            TurnDown(data3.get(), row, col, d_posx, d_posy);
            if (Prune(data3.get(), row, col) && CleanRoom(data3.get(), row, col, d_posx, d_posy, path + "d")) {
                return true;
            }
        }

        d_posx = posx;
        d_posy = posy;
        if (canTurnLeft) {
            TurnLeft(data4.get(), row, col, d_posx, d_posy);
            if (Prune(data4.get(), row, col) && CleanRoom(data4.get(), row, col, d_posx, d_posy, path + "l")) {
                return true;
            }
        }

        if (!canTurnUp && !canTurnDown && !canTurnLeft && !canTurnRight) {
            result = CheckFinished(data.get(), map_size);
            if (result) {
                m_path = path;
            }
        }

        return result;
    }

    void DoSolveProblem(int id = 0)
    {
        while (!got_map) {
            cout << "Getting Map ..." << endl;
        }

        int size = col * row;
        PrintMap(map.get(), row, col);

        int idx = id;
        for (idx = id; idx < size; idx += 2) {
            if (map.get()[idx] == 0) {
                start_posx = idx % col;
                start_posy = idx / col;
                break;
            }
        }

        while (!CleanRoom(map.get(), row, col, start_posx, start_posy)) {
            cout << "thread_id =" << this_thread::get_id() << endl;
            cout << "start_posx = " << start_posx << " start_posy = " << start_posy << endl;
            if ((idx += 2) >= size) {
                break;
            }
            for (; idx < size; idx += 2) {
                if (map.get()[idx] == 0) {
                    start_posx = idx % col;
                    start_posy = idx / col;
                    break;
                }
            }
        }

        PrintSolution();

        PostSolution();

        got_map = false;
    }

    void PrintSolution(void)
    {
        cout << "start_posx =" << start_posx << " start_posy =" << start_posy << endl;
        cout << "path = " << m_path << endl;
    }

    static void SetMap(const std::string &path)
    {
        map = shared_ptr<char>(new char[path.length()]);
        memset(map.get(), 0, path.length());

        for (int i = 0 ; i < path.length() ; ++i) {
            if (path.at(i) == '1') {
                map.get()[i] = 255;
            }
        }
    }

private:
    CURL *handler = nullptr;
    string get_url;
    string post_url;

    static shared_ptr<char> map;
    static int  row;
    static int  col;
    static volatile bool got_map;

    int start_posx = 0;
    int start_posy = 0;

    string m_path;

    /// Network functions
    static size_t GetMapWriteDat(void *buffer, size_t size, size_t nmemb, void *userp)
    {
        (void) userp;

        string src = string(static_cast<char *>(buffer));
        regex match_map("level=\\d+&x=\\d+&y=\\d+&map=\\d+", regex_constants::icase);

        auto smatch = sregex_iterator(src.begin(), src.end(), match_map);
        auto match_end = sregex_iterator();

        if (smatch != match_end) {
            string match_string = (*smatch).str();

            regex row_match("x=\\d+");
            regex col_match("y=\\d+");
            regex map_match("map=\\d+");

            auto col_match_iter = sregex_iterator(match_string.begin(), match_string.end(), col_match);
            string col_string = (*col_match_iter).str();

            col_string.erase(0, 2);
            col = stol(col_string);

            auto row_match_iter = sregex_iterator(match_string.begin(), match_string.end(), row_match);
            string row_string = (*row_match_iter).str();

            row_string.erase(0, 2);
            row = stol(row_string);

            auto map_match_iter = sregex_iterator(match_string.begin(), match_string.end(), map_match);
            string map_string = (*map_match_iter).str();

            map_string.erase(0, 4);

            map = shared_ptr<char>(new char[map_string.length()]);
            memset(map.get(), 0, map_string.length());

            for (int i = 0 ; i < map_string.length() ; ++i) {
                if (map_string.at(i) == '1') {
                    map.get()[i] = 255;
                }
            }

            got_map = true;
        }

        return size * nmemb;
    }

    static size_t PostDataFunction(void *buffer, size_t size, size_t nmemb, void *userp)
    {
        (void) buffer;
        (void) userp;

        cout << endl;
        for ( int i = 0 ; i < size * nmemb ; ++i ) {
            cout << static_cast<char *>(buffer)[i];
        }
        cout << endl;

        return size * nmemb;
    }

    // Core solution functions
    bool CanTurnUp(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posy > 0) && (*(map + (posy - 1) * col + posx) == 0));
    }

    void TurnUp(char *map, const int &row, const int &col, const int &posx, int &posy)
    {
        int py = 0;
        for (py = posy - 1; py >= 0; --py) {
            if (!*(map + py * col + posx)) {
                *(map + py * col + posx) =  1;
            } else {
                posy = py + 1;
                break;
            }
        }
        if (py == -1) {
            posy = 0;
        }
    }

    bool CanTurnDown(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posy < row - 1) && (*(map + (posy + 1) * col + posx) == 0));
    }

    void TurnDown(char *map, const int &row, const int &col, const int &posx, int &posy)
    {
        int py = 0;
        for (py = posy + 1; py < row ; ++py) {
            if (!*(map + py * col + posx)) {
                *(map + py * col + posx) =  1;
            } else {
                posy = py - 1;
                break;
            }
        }
        if (py == row) {
            posy = py - 1;
        }
    }

    bool CanTurnLeft(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posx > 0) && (*(map + posy * col + posx - 1) == 0));
    }

    void TurnLeft(char *map, const int &row, const int &col, int &posx, const int &posy)
    {
        int px = 0;
        for (px = posx - 1; px >= 0 ; --px) {
            if (!*(map + posy * col + px)) {
                *(map + posy * col + px) =  1;
            } else {
                posx = px + 1;
                break;
            }
        }

        if (px == -1) {
            posx = 0;
        }
    }

    bool CanTurnRight(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posx < col - 1) && (*(map + posy * col + posx + 1) == 0));
    }

    void TurnRight(char *map, const int &row, const int &col, int &posx, const int &posy)
    {
        int px = 0;
        for (px = posx + 1 ; px < col; ++px) {
            if (*(map + posy * col + px) == 0) {
                *(map + posy * col + px) =  1;
            } else {
                posx = px - 1;
                break;
            }
        }
        if (px == col) {
            posx = px - 1;
        }
    }

    bool CheckFinished(const char *map, int size)
    {
        for (int i = 0; i < size; ++i) {
            if (!map[i]) {
                return false;
            }
        }

        return true;
    }

    void PrintMap(char *map, int row, int col)
    {
        cout << endl << endl << "====== PrintMap =========" << endl;
        cout << "row =" << row << " col =" << col << endl;
        cout << "start_posx =" << start_posx << " start_posy =" << start_posy << endl;
        cout << endl;
        for ( int r = 0  ; r < row ; ++r ) {
            for ( int c = 0 ; c < col ; ++c ) {
                char data = *(map + r * col + c);
                if (data == -1) {
                    cout << "#";
                } else if (data == 1) {
                    cout << "*";
                } else if (data == 2) {
                    cout << "$";
                } else {
                    cout << ".";
                }
            }
            cout << endl;
        }
        cout << "++++++ PrintMap ++++++++" << endl << endl << endl;
    }

    void FloodFill(char *map, const int row, const int col, int posx, int posy)
    {
        *(map + posy * col + posx) = 2;

        if (posx > 0 && *(map + posy * col + posx - 1) == 0) {
            FloodFill(map, row, col, posx - 1, posy);
        }

        if (posy > 0 && *(map + (posy - 1) * col + posx) == 0) {
            FloodFill(map, row, col, posx, posy - 1);
        }

        if (posx < col && *(map + posy * col + posx + 1) == 0) {
            FloodFill(map, row, col, posx + 1, posy);
        }

        if (posy < row && *(map + (posy + 1) * col + posx) == 0) {
            FloodFill(map, row, col, posx , posy + 1);
        }
    }

    bool Prune(const char *map, const int row, const int col)
    {
        return true;

        int size = row * col;
        int posx = 0;
        int posy = 0;
        int i = 0;
        for (i = 0; i < size; ++i) {
            if (map[i] == 0) {
                posx = i % col;
                posy = i / col;
                break;
            }
        }

        char fill_data[size];
        memcpy(fill_data, map, size);

        FloodFill(fill_data, row, col, posx, posy);

        bool result = CheckFinished(fill_data, size);

        if (!result) {
            cout << "FloodFill result =" << result << endl;
            PrintMap(fill_data, row, col);
        }

        return result;
    }
};

shared_ptr<char> Solution::map;
int Solution::row = 0;
int Solution::col = 0;
volatile bool Solution::got_map = false;

int
main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);

    Solution *solution = new Solution("http://www.qlcoder.com/train/autocr", "http://www.qlcoder.com/train/crcheck");

    solution->SetCookies("laravel_session=eyJpdiI6IkhFckdFZkZOam9wOW9IRmxNSXBaMWc9PSIsInZhbHVlIjoiZFRuT1A2d0V5bjNpMGZVTXYyRGJYZlhQQUhLbjRlNVZ1Q2U1MDhLTkdtRStUZHR3UmdPY3NRdWNFYUNqNEJxRFhGdmM1aWtQc2FMd1g1Wk1NQXNvUUE9PSIsIm1hYyI6ImM1NWJjY2NjNTZiNTM1NzU0MTg1MWEyNzliMmVkZmRhOTU3NjI3MmUxYjg0OTkwOWI0MjhmN2ZjMjM0NTFkYjIifQ%3D%3D;");

    for (;;) {
        solution->GetMap();

        thread t1(&Solution::DoSolveProblem, solution, 0);
        thread t2(&Solution::DoSolveProblem, solution, 1);
        t1.join();
        t2.join();

        // solution->PostSolution();
    }

    curl_global_cleanup();
    return 0;
}


