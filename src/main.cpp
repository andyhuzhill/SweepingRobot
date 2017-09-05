#include <iostream>
using namespace std;

#include <Poco/Net/NetException.h>

#include "solution.hpp"

int main() {
    try {
        Solution solution;
        NameValueCollection cookies;
        cookies.add(
            "laravel_session",
            "eyJpdiI6IjlHd0p5OUtkVWV3V25KY1JMWld0enc9PSIsInZhbHVlIjoiWUUyQ0"
            "JEWWJPZ2Fub1ZFbWRHWWF5QUlhV3B1Uk9jNElqT3k2bjFmZ2FXNkFIK3VPb3k4"
            "dHVGTDM0dFl0dWF6a1ZCUzZ5dmUyb0pUSWc0cXQ4NDZ0RVE9PSIsIm1hYyI6Ij"
            "ZlYzBlZTdiZGRmNWUzOGJkYTkzYWYyOGVhODQzOWQxOWY3MDNjYjJjMGEzNjM5"
            "NjNlMTRiNjljYTg0MjEyYzAifQ%3D%3D");
        solution.setCookies(cookies);

        solution.run();
    } catch (NetException& ex) {
        cout << "exception = " << ex.what() << endl;
    }

    int ch;
    cin >> ch;

    return 0;
}
