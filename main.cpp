#include <iostream>
#include <iomanip>
#include <regex>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <cstdint>
#include <exception>

#include <Poco/StreamCopier.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTMLForm.h>

#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

class TaskWalk :public Runnable
{
public:
    TaskWalk() {}

    void run() final {
        cout << "Hello World!" << endl;
    }
};

class MapData
{
public:
    MapData(int level, int w, int h, string mapstr = "") {
        m_level = level;
        m_width = w;
        m_height = h;
        data = shared_ptr<int8_t>(new int8_t[w * h]);
        memset(data.get(), 0, w * h);
        if (!mapstr.empty()) {
            for (int i = 0; i < mapstr.length(); ++i) {
                data.get()[i] = mapstr.at(i) - 0x30;
            }
        }
    }

    const int level() {
        return m_level;
    }

    const int width() {
        return m_width;
    }

    const int height() {
        return m_height;
    }

    int8_t* const operator[](int row) {
        return &(data.get()[row * m_width]);
    }

    int8_t& at(int row, int col) {
        assert(row >= 0);
        assert(row < m_height);
        assert(col >= 0);
        assert(col < m_width);
        return data.get()[row * m_width + col];
    }

    /** 检查坐标 (col, row) 周围有多少空位 */
    int space(int row, int col) {
        assert(row >= 0);
        assert(row < m_height);
        assert(col >= 0);
        assert(col < m_width);

        int result = 0;

        result += (row == 0) ? 0 : (at(row -1, col) == 0);
        result += (row == m_height - 1) ? 0 : (at(row + 1, col) == 0);
        result += (col == 0) ? 0 : (at(row, col - 1) == 0);
        result += (col == m_width - 1) ? 0 : (at(row, col + 1) == 0);

        return result;
    }

    void print() {
        cout << "level = " << m_level << endl;
        cout << "row = " << m_height << endl;
        cout << "col = " << m_width << endl;
        for (int i = 0; i < m_height; ++i) {
            cout << "row " << setw(2) << i << ": ";
            for (int j = 0; j < m_width; ++j) {
                cout << int(data.get()[i * m_width + j]);
            }
            cout << endl;
        }
    }

private:
    int m_level = 0;
    int m_width = 0;
    int m_height = 0;
    shared_ptr<int8_t> data;
};

class Solution
{

public:
    Solution() {
        session.setHost("www.qlcoder.com");
    }

    void getMap(NameValueCollection cookies) {
        HTTPRequest getRequest(HTTPRequest::HTTP_GET, "/train/autocr");
        getRequest.setCookies(cookies);

        session.sendRequest(getRequest);

        HTTPResponse response;
        istream &is = session.receiveResponse(response);
        string responseHTML;
        StreamCopier::copyToString(is, responseHTML);

        regex re(R"(level=(\d+)&x=(\d+)&y=(\d+)&map=(\d+))");

        auto begin = sregex_iterator(responseHTML.begin(), responseHTML.end(), re);
        if (begin != sregex_iterator()) {
            smatch match = (*begin);
            int level = stoi(match.str(1));
            int row = stoi(match.str(2)); // row height
            int col = stoi(match.str(3)); // col width
            string mapstr = match.str(4);

            MapData mapdata(level, col, row, mapstr);
            mapdata.print();

            for (int i = 0; i < row; ++i) {
                for (int j = 0; j < col; ++j) {
                    if (mapdata.at(i, j) == 0) {
                        cout << "row:" << setw(2) << i << " col:" << setw(2) << j << " :" << mapdata.space(i, j) << endl;
                    }
                }
            }
        }
    }
private:
    HTTPClientSession session;

};

int main()
{
    try {
        Solution solution;
        NameValueCollection cookies;
        cookies.add("laravel_session", "eyJpdiI6IjlHd0p5OUtkVWV3V25KY1JMWld0enc9PSIsInZhbHVlIjoiWUUyQ0JEWWJPZ2Fub1ZFbWRHWWF5QUlhV3B1Uk9jNElqT3k2bjFmZ2FXNkFIK3VPb3k4dHVGTDM0dFl0dWF6a1ZCUzZ5dmUyb0pUSWc0cXQ4NDZ0RVE9PSIsIm1hYyI6IjZlYzBlZTdiZGRmNWUzOGJkYTkzYWYyOGVhODQzOWQxOWY3MDNjYjJjMGEzNjM5NjNlMTRiNjljYTg0MjEyYzAifQ%3D%3D");
        solution.getMap(cookies);
    } catch (exception e) {
        cout << "e = " << e.what() << endl;
    }

    return 0;
}

