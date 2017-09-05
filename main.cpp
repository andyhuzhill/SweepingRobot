#include <iostream>
#include <iomanip>
#include <regex>
#include <string>
#include <stack>
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

typedef struct {
    int x;
    int y;
} Position;

class MapData
{
public:
    MapData() {
    }

    MapData(int level, int w, int h, string mapstr = "") {
        m_level = level;
        m_width = w;
        m_height = h;
        data = shared_ptr<uint8_t>(new uint8_t[w * h]);
        memset(data.get(), 0, w * h);
        if (!mapstr.empty()) {
            for (int i = 0; i < mapstr.length(); ++i) {
                data.get()[i] = mapstr.at(i) - 0x30;
            }
        }
    }

    //    MapData(const MapData &src) {
    //        m_level = src.level();
    //        m_width = src.width();
    //        m_height = src.height();
    //        data = shared_ptr<uint8_t>(new uint8_t[m_width * m_height]);
    //        memcpy(data.get(), src.getData(), m_width * m_height);
    //    }

    //    MapData &operator=( MapData &src) {
    //        if (this != &src) {
    //            m_level = src.level();
    //            m_width = src.width();
    //            m_height = src.height();
    //            data = shared_ptr<uint8_t>(new uint8_t[m_width * m_height]);
    //            memcpy(data.get(), src.getData(), m_width * m_height);
    //        }
    //        return *this;
    //    }

    int level() {
        return m_level;
    }

    int width() {
        return m_width;
    }

    int height() {
        return m_height;
    }

    uint8_t* const operator[](int row) {
        return &(data.get()[row * m_width]);
    }

    uint8_t& at(int row, int col) {
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

    string getResult() {
        int resultLength = m_path.size() - 1;
        shared_ptr<char> resultBuff = shared_ptr<char>(new char[resultLength]);
        memset(resultBuff.get(), 0, resultLength);

        if (m_path.size() > 1) {
            Position last = m_path.top();
            m_path.pop();
            int pos = resultLength - 1;

            while (!m_path.empty()) {
                Position curr = m_path.top();

                if (curr.x == last.x) {
                    if (curr.y > last.y) {
                        resultBuff.get()[pos--] = 'l';
                    } else {
                        resultBuff.get()[pos--] = 'r';
                    }
                } else if (curr.y == last.y) {
                    if (curr.x > last.x) {
                        resultBuff.get()[pos--] = 'd';
                    } else {
                        resultBuff.get()[pos--] = 'u';
                    }
                }

                last = curr;
            }
        }

        return string(resultBuff.get());
    }

    bool isFinished(){
        bool result = true;
        for (int i = 0; i < m_width * m_height; ++i) {
            if (int(data.get()[i]) == 0) {
                return false;
            }
        }
        return result;
    }

    uint8_t *getData() {
        return data.get();
    }

private:
    int m_level = 0;
    int m_width = 0;
    int m_height = 0;
    shared_ptr<uint8_t> data;

    stack<Position> m_path;
};

class Solution
{
public:
    Solution() {
        session.setHost("www.qlcoder.com");
    }

    const MapData& getMap() {
        HTTPRequest getRequest(HTTPRequest::HTTP_GET, "/train/autocr");
        getRequest.setCookies(m_cookies);

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

            return move(MapData(level, col, row, mapstr));
        } else {
            return move(MapData());
        }
    }

    bool postResult() {
        // TODO
        return true;
    }

    void setCookies(const NameValueCollection &cookies){
        m_cookies = cookies;
    }

    void run() {
        auto map = getMap();
        map.print();
    }

private:
    HTTPClientSession session;

    NameValueCollection m_cookies;

    const MapData m_map;
};

int main()
{
    try {
        Solution solution;
        NameValueCollection cookies;
        cookies.add("laravel_session", "eyJpdiI6IjlHd0p5OUtkVWV3V25KY1JMWld0enc9PSIsInZhbHVlIjoiWUUyQ0JEWWJPZ2Fub1ZFbWRHWWF5QUlhV3B1Uk9jNElqT3k2bjFmZ2FXNkFIK3VPb3k4dHVGTDM0dFl0dWF6a1ZCUzZ5dmUyb0pUSWc0cXQ4NDZ0RVE9PSIsIm1hYyI6IjZlYzBlZTdiZGRmNWUzOGJkYTkzYWYyOGVhODQzOWQxOWY3MDNjYjJjMGEzNjM5NjNlMTRiNjljYTg0MjEyYzAifQ%3D%3D");
        solution.setCookies(cookies);

        solution.run();
    } catch (exception e) {
        cout << "e = " << e.what() << endl;
    }

    return 0;
}
