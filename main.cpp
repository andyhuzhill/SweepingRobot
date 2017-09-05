#include <iostream>
#include <regex>
#include <string>
#include <vector>

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
            string level = match.str(1);
            int x = stoi(match.str(2));
            int y = stoi(match.str(3));
            string map = match.str(4);

            cout << "level = " << level << endl;
            cout << "x = " << x << endl;
            cout << "y = " << y << endl;
            cout << "map = " << map << endl;
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
    } catch (Exception e) {
        cout << "e = " << e.what() << endl;
    }

    return 0;
}

