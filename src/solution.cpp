#include <assert.h>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <Poco/StreamCopier.h>

#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>

#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>

#include "solution.hpp"

/////////////////////////////////////////////////////
// MapData
/////////////////////////////////////////////////////

MapData::MapData() {}

MapData::MapData(int level, int w, int h, string mapstr) {
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

int MapData::level() { return m_level; }

int MapData::width() { return m_width; }

int MapData::height() { return m_height; }

uint8_t* const MapData::operator[](int row) {
    return &(data.get()[row * m_width]);
}

uint8_t& MapData::at(int row, int col) {
    assert(row >= 0);
    assert(row < m_height);
    assert(col >= 0);
    assert(col < m_width);
    return data.get()[row * m_width + col];
}

int MapData::space(int row, int col) {
    assert(row >= 0);
    assert(row < m_height);
    assert(col >= 0);
    assert(col < m_width);

    int result = 0;

    result += (row == 0) ? 0 : (at(row - 1, col) == 0);
    result += (row == m_height - 1) ? 0 : (at(row + 1, col) == 0);
    result += (col == 0) ? 0 : (at(row, col - 1) == 0);
    result += (col == m_width - 1) ? 0 : (at(row, col + 1) == 0);

    return result;
}

void MapData::print() {
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

bool MapData::isFinished() {
    for (int i = 0; i < m_width * m_height; ++i) {
        if (int(data.get()[i]) == 0) {
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////
// Solution
/////////////////////////////////////////////////////

Solution::Solution() { session.setHost("www.qlcoder.com"); }

void Solution::setCookies(const NameValueCollection& cookies) {
    m_cookies = cookies;
}

const MapData& Solution::getMap() {
    HTTPRequest getRequest(HTTPRequest::HTTP_GET, "/train/autocr");
    getRequest.setCookies(m_cookies);

    session.sendRequest(getRequest);

    HTTPResponse response;
    istream& is = session.receiveResponse(response);
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

bool Solution::postResult() {
    HTTPRequest postRequest(HTTPRequest::HTTP_POST, "/train/crcheck");
    postRequest.setCookies(m_cookies);

    HTMLForm form;
    form.add("x", to_string(startPosX));
    form.add("y", to_string(startPosY));
    string path = getResult();
    form.add("path", path);

    cout << "solution: start_x = " << startPosX << " start_y = " << startPosY
         << " path = " << path << endl;

    form.prepareSubmit(postRequest);

    form.write(session.sendRequest(postRequest));

    HTTPResponse response;
    auto& is = session.receiveResponse(response);
    string responseHTML;
    StreamCopier::copyToString(is, responseHTML);

    return true;
}

string Solution::getResult() {
    if (m_path.size() > 1) {
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
    } else {
        return "";
    }
}

void Solution::run() {
    for (;;) {
        auto map = getMap();
        map.print();

        postResult();
    }
}