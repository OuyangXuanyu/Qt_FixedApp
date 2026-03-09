//
// Created by Administrator on 26-1-13.
//

#include "user_database.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "../diy_encryption/diy_encryption.h"

UserInfoDatabase::UserInfoDatabase(const std::string& appPath) {
    const std::string dbPath = appPath + "/UserInfo.db";
    std::cout<<dbPath<<std::endl;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open UserInfo.db");
    }

    const auto sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "account TEXT NOT NULL,"
        "password TEXT NOT NULL,"
        "usertype TEXT CHECK(usertype IN ('doctor','nurse','debugger'))"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error(err);
    }
}

UserInfoDatabase::~UserInfoDatabase() {
    close();
}

void UserInfoDatabase::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

int UserInfoDatabase::findUser(const QString& account,
                               const QString& password,
                               const QString& usertype)
{
    const auto sql =
        "SELECT id FROM users "
        "WHERE account = ? AND password = ? AND usertype = ?";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, account.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashPasswordQt(password).toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, usertype.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    int userID = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userID = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return userID;  // -1 表示不存在
}

bool UserInfoDatabase::signup(const QString& account,
                              const QString& password,
                              const QString& usertype)
{
    if (findUser(account, password, usertype) != -1)
        return false;

    const auto sql =
        "INSERT INTO users (account, password, usertype) "
        "VALUES (?, ?, ?)";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, account.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashPasswordQt(password).toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, usertype.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    return ok;
}

int UserInfoDatabase::login(const QString& account,
                            const QString& password,
                            const QString& usertype)
{
    const int uid = findUser(account, password, usertype);
    return uid == -1 ? 0 : uid;
}

int UserInfoDatabase::deleteUser(const QString& account,
                                 const QString& usertype)
{
    const auto sql =
        "DELETE FROM users WHERE account = ? AND usertype = ?";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, account.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, usertype.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    const int rows = sqlite3_changes(db);

    sqlite3_finalize(stmt);
    return rows;
}

const char *usertype_to_string(const uint8_t usertype) {
    switch (usertype) {
        case 0: return "doctor";
        case 1: return "nurse";
        case 2: return "debugger";
        default: return "unknown";
    }
}

