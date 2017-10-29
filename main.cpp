/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  robot
 *
 *        Version:  1.0
 *       Revision:  none
 *       Compiler:  g++ 5.3.1
 *       Compile Flags: -std=c++11 -Wall -lcurl
 *
 * =====================================================================================
 */

#include <iostream>
#include <curl/curl.h>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <unistd.h>

using namespace std;

class Solution
{
public:
    Solution(const string &get, const string &post)
    {
        handler = curl_easy_init();
        curl_easy_setopt(handler, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(handler, CURLOPT_PROGRESSFUNCTION, GetMapProgressFunc);
        curl_easy_setopt(handler, CURLOPT_PROGRESSDATA, "Flag");

        get_url = get;
        post_url = post;
    }

    static void SetCookies(const string &cookie)
    {
        curl_easy_setopt(handler, CURLOPT_COOKIE, cookie.c_str());
    }

    static void GetMap()
    {
        got_map = false;
        got_answer = false;
        m_path = "";
        map_str = "";

        curl_easy_setopt(handler, CURLOPT_URL, get_url.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, Solution::GetMapWriteDat);

        CURLcode code = curl_easy_perform(handler);
        if (code != CURLE_OK) {
            cerr << "Can't perform Get operations!" << endl;
        }
    }

    static void PostSolution()
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

    static bool CleanRoom(const char *map, const int &row, const int &col, int posx, int posy, const string &path = "")
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
                got_answer = true;
            }
        }

        return result;
    }

    static void ThreadTask(int start_idx, int end_idx)
    {
        cout << "start_idx =" << start_idx << " end_idx=" << end_idx << endl;

        int start_pos_x = 0;
        int start_pos_y = 0;
        int idx = start_idx;
        for (; idx < end_idx; idx++ ) {
            if (map.get()[idx] == 0) {
                start_pos_x = idx % col;
                start_pos_y = idx / col;
                break;
            }
        }

        while (!got_answer) {
            if (!CleanRoom(map.get(), row, col, start_pos_x, start_pos_y)) { // solution not found
                cout << "start_pos_x =" << start_pos_x << " start_pos_y = " << start_pos_y << endl;
                if ((idx++) >= end_idx) {
                    break;
                }
                for (; idx < end_idx; idx++) {
                    if (map.get()[idx] == 0) {
                        start_pos_x = idx % col;
                        start_pos_y = idx / col;
                        break;
                    }
                }
            } else { // Found Solution !!
                start_posx = start_pos_x;
                start_posy = start_pos_y;
                PrintSolution();
                PostSolution();
            }
        }

        cout << "thread " << this_thread::get_id() << " quit!" << endl;
    }

    static void DoSolveProblem(int thread_cnt = 1)
    {
        while (!got_map) {
            cout << "Getting Map ..." << endl;
        }

        PrintMap(map.get(), row, col);

        thread thread_pool[thread_cnt];

        int idx_array[thread_cnt + 1];
        memset(idx_array, 0, thread_cnt + 1);

        int map_size = row * col;
        cout << "level =" << level << endl;

        for (int i = 0; i < thread_cnt; ++i) {
            idx_array[i] = i * map_size / thread_cnt;
        }
        idx_array[thread_cnt] = map_size;

        for (int i = 0; i < thread_cnt ; ++i) {
            thread_pool[i] = thread(Solution::ThreadTask, idx_array[i], idx_array[i + 1]);
        }

        for (auto &thrd : thread_pool) {
            if (thrd.joinable()) {
                thrd.join();
            }
        }

        if (!got_answer) {
            cout << "Solution Not Found!" << endl;
            while (1);
        }

        got_map = false;
    }

    static void PrintSolution(void)
    {
        cout << "start_posx =" << start_posx << " start_posy =" << start_posy << endl;
        cout << "path = " << m_path << endl;
    }

    static void SetMap(const string &path)
    {
        map = shared_ptr<char>(new char[path.length()]);
        memset(map.get(), 0, path.length());

        for (uint i = 0 ; i < path.length() ; ++i) {
            if (path.at(i) == '1') {
                map.get()[i] = 255;
            }
        }
    }

private:
    static CURL *handler;
    static string get_url;
    static string post_url;
    static string map_str;

    static shared_ptr<char> map;
    static int  row;
    static int  col;
    static volatile bool got_map;
    static volatile bool got_answer;
    static int level;

    static int start_posx;
    static int start_posy;

    static string m_path;

    /// Network functions
    //
    static size_t GetMapProgressFunc(char *bar, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        if (isinf(dlnow * 100.0 / dltotal)) {
            if (got_map) {
                return 0;
            }
            string src = map_str;

            regex match_map("level=\\d+&x=\\d+&y=\\d+&map=\\d+", regex_constants::icase);

            auto smatch = sregex_iterator(src.begin(), src.end(), match_map);
            auto match_end = sregex_iterator();

            if (smatch != match_end) {
                string match_string = (*smatch).str();

                regex level_match("level=\\d+");
                regex row_match("x=\\d+");
                regex col_match("y=\\d+");
                regex map_match("map=\\d+");

                auto level_match_iter = sregex_iterator(match_string.begin(), match_string.end(),  level_match);
                string level_string = (*level_match_iter).str();

                level_string.erase(0, 6);
                level = stol(level_string);

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

                for (uint i = 0 ; i < map_string.length() ; ++i) {
                    if (map_string.at(i) == '1') {
                        map.get()[i] = 255;
                    }
                }

                got_map = true;
            }

        }
        return 0;
    }

    static size_t GetMapWriteDat(void *buffer, size_t size, size_t nmemb, void *userp)
    {
        (void) userp;

        string src = string(static_cast<char *>(buffer));
        map_str += src;

        return size * nmemb;
    }

    static size_t PostDataFunction(void *buffer, size_t size, size_t nmemb, void *userp)
    {
        (void) buffer;
        (void) userp;

        cout << endl;
        for (uint i = 0 ; i < size * nmemb ; ++i) {
            cout << static_cast<char *>(buffer)[i];
        }
        cout << endl;

        return size * nmemb;
    }

    // Core solution functions
    static bool CanTurnUp(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posy > 0) && (*(map + (posy - 1) * col + posx) == 0));
    }

    static void TurnUp(char *map, const int &row, const int &col, const int &posx, int &posy)
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

    static bool CanTurnDown(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posy < row - 1) && (*(map + (posy + 1) * col + posx) == 0));
    }

    static void TurnDown(char *map, const int &row, const int &col, const int &posx, int &posy)
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

    static bool CanTurnLeft(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posx > 0) && (*(map + posy * col + posx - 1) == 0));
    }

    static void TurnLeft(char *map, const int &row, const int &col, int &posx, const int &posy)
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

    static bool CanTurnRight(const char *map, const int &row, const int &col, const int posx, const int posy)
    {
        return ((posx < col - 1) && (*(map + posy * col + posx + 1) == 0));
    }

    static void TurnRight(char *map, const int &row, const int &col, int &posx, const int &posy)
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

    static bool CheckFinished(const char *map, int size)
    {
        for (int i = 0; i < size; ++i) {
            if (!map[i]) {
                return false;
            }
        }

        return true;
    }

    static void PrintMap(char *map, int row, int col)
    {
        cout << endl << endl << "====== PrintMap =========" << endl;
        cout << "row =" << row << " col =" << col << endl;
        cout << endl;

        for (int r = 0  ; r < row ; ++r) {
            for (int c = 0 ; c < col ; ++c) {
                char data = *(map + r * col + c);
#if 1
                if (data == -1) {
                    cout << "#";
                } else if (data == 1) {
                    cout << "*";
                } else if (data == 2) {
                    cout << "$";
                } else {
                    cout << ".";
                }
#endif
            }
            cout << endl;
        }

        cout << "++++++ PrintMap ++++++++" << endl << endl << endl;
    }

    static void FloodFill(char *map, const int row, const int col, int posx, int posy)
    {
        *(map + posy * col + posx) = 2;

        if (posx > 0 && *(map + posy * col + posx - 1) == 0) {
            FloodFill(map, row, col, posx - 1, posy);
        }

        if (posy > 0 && *(map + (posy - 1) * col + posx) == 0) {
            FloodFill(map, row, col, posx, posy - 1);
        }

        if (posx < (col - 1) && *(map + posy * col + posx + 1) == 0) {
            FloodFill(map, row, col, posx + 1, posy);
        }

        if (posy < (row - 1) && *(map + (posy + 1) * col + posx) == 0) {
            FloodFill(map, row, col, posx , posy + 1);
        }
    }

    static bool Prune(const char *map, const int row, const int col)
    {
        int size = row * col;
        int posx = 0;
        int posy = 0;
        for (int i = 0; i < size; ++i) {
            if (map[i] == 0) {
                posx = i % col;
                posy = i / col;
                break;
            }
        }

        char fill_data[size];
        memcpy(fill_data, map, size);

        FloodFill(fill_data, row, col, posx, posy);

        return CheckFinished(fill_data, size);
    }
};

CURL *Solution::handler = nullptr;
string Solution::get_url;
string Solution::post_url;
string Solution::map_str;

shared_ptr<char> Solution::map;
int Solution::row = 0;
int Solution::col = 0;
volatile bool Solution::got_map = false;
volatile bool Solution::got_answer = false;
int Solution::level = 0;
int Solution::start_posx = 0;
int Solution::start_posy = 0;
string Solution::m_path;

int
main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);

    Solution *solution = new Solution("http://www.qlcoder.com/train/autocr", "http://www.qlcoder.com/train/crcheck");

    solution->SetCookies("laravel_session=eyJpdiI6IkhFckdFZkZOam9wOW9IRmxNSXBaMWc9PSIsInZhbHVlIjoiZFRuT1A2d0V5bjNpMGZVTXYyRGJYZlhQQUhLbjRlNVZ1Q2U1MDhLTkdtRStUZHR3UmdPY3NRdWNFYUNqNEJxRFhGdmM1aWtQc2FMd1g1Wk1NQXNvUUE9PSIsIm1hYyI6ImM1NWJjY2NjNTZiNTM1NzU0MTg1MWEyNzliMmVkZmRhOTU3NjI3MmUxYjg0OTkwOWI0MjhmN2ZjMjM0NTFkYjIifQ%3D%3D;");

    for (;;) {
        solution->GetMap();

        sleep(5);

        solution->DoSolveProblem(8);
    }

    curl_global_cleanup();
    return 0;
}
