#pragma once
#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <memory>
#include <stack>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/NameValueCollection.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

typedef struct {
    int x;
    int y;
} Position;

class MapData {
  public:
    explicit MapData();

    explicit MapData(int level, int w, int h, string mapstr = "");

    int level();
    int width();
    int height();

    uint8_t* const operator[](int row);

    // 返回第row行第col列的点
    uint8_t& at(int row, int col);

    /** 检查坐标 (col, row) 周围有多少空位 */
    int space(int row, int col);

    // 打印出地图
    void print();

    // 判断是否已经全部清扫完毕
    bool isFinished();

  private:
    int m_level = 0;
    int m_width = 0;
    int m_height = 0;
    shared_ptr<uint8_t> data;
};

class Solution {
  public:
    explicit Solution();

    // 设置 Cookies
    void setCookies(const NameValueCollection& cookies);

    // 获取地图
    const MapData& getMap();

    // 提交结果
    bool postResult();

    // 获取计算出来的路径
    string getResult();

    void run();

  private:
    HTTPClientSession session;

    NameValueCollection m_cookies;

    int startPosX = 0;
    int startPosY = 0;
    stack<Position> m_path;
};

#endif