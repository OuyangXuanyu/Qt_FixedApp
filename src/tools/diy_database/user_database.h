//
// Created by Administrator on 26-1-13.
//

#ifndef USER_DATABASE_H
#define USER_DATABASE_H

#include <QString>
#include <string>
#include <vector>
#include "../../third_src/sqlite3/sqlite3.h"

class UserInfoDatabase {
public:
    explicit UserInfoDatabase(const std::string& appPath);
    ~UserInfoDatabase();

    int findUser(const QString& account,
                 const QString& password,
                 const QString& usertype);

    bool signup(const QString& account,
                const QString& password,
                const QString& usertype);

    int login(const QString& account,
              const QString& password,
              const QString& usertype);

    int deleteUser(const QString& account,
                   const QString& usertype);

    void close();

    // 调试用
    std::vector<std::vector<std::string>> testShow();

private:
    sqlite3* db{};
};

const char* usertype_to_string(uint8_t);

#endif //USER_DATABASE_H
