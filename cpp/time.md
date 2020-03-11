# 时间模块

## 高精度计时器
- 获取当前时间的毫秒表示
    ```cpp
    long long CELLTime::getNowInMilliseconds()
    {
        return duration_cast<milliseconds>
            (high_resolution_clock::now().time_since_epoch()).count();
    }

- 获取当前时间的年、月、日。
    ```cpp
    std::string CELLTime::getNowInStr()
    {
        auto t = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::to_time_t(t);
        std::tm *tNow = std::localtime(&now);
        char ret[32] = {};
        sprintf(ret, "%d-%02d-%02d %02d:%02d:%02d",
            tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday,
            tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
        return std::string(ret);
    }

## 获取时间差
- 获取高精度计时器
    ```cpp
    void CELLTimeStamp::update()
    {
        //time_point<high_resolution_clock> _begin;
        _begin = high_resolution_clock::now();
    }
- 从指定时间起，经过的微妙
    ```cpp
    long long CELLTimeStamp::getElapseTimeInMicroSeconds()
    {
        return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
    }