//
// Created by Administrator on 26-1-13.
//

#include "patient_database.h"
#include <iostream>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <utility>

using Row = std::map<std::string, std::string>;

PatientDatabase::PatientDatabase(std::string appPath) : appPath(std::move(appPath))
{
    const std::string dbPath = this->appPath + "/Patient.db";
    // std::cout<<"pd路径: "<<dbPath<<std::endl;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database");
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS Patient_Info ("
        "patient_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "patient_id_diy INTEGER NOT NULL UNIQUE,"
        "patient_name TEXT NOT NULL,"
        "gender TEXT,"
        "age INTEGER,"
        "height_cm REAL,"
        "weight_kg REAL,"
        "blood_pressure_sys REAL,"
        "blood_pressure_dia REAL,"
        "clinical_diagnosis TEXT,"
        "invasive_icp REAL,"
        "operator TEXT NOT NULL,"
        "operator_ID INTEGER NOT NULL,"
        "create_time DATETIME DEFAULT (strftime('%Y%m%d%H%M%S','now','localtime')),"
        "update_time DATETIME DEFAULT (strftime('%Y%m%d%H%M%S','now','localtime'))"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error(err);
    } else {
        return;
        // std::cout << "Patient database successfully created" << std::endl;
    }
}

PatientDatabase::~PatientDatabase() {
    close();
}

void PatientDatabase::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool PatientDatabase::isDigitString(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

std::string PatientDatabase::nowTime() {
    char buf[32];
    std::time_t t = std::time(nullptr);
    std::strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", std::localtime(&t));
    return buf;
}

bool PatientDatabase::searchPatient(
    const std::string& info,
    std::vector<Row>& result)
{
    if (!db) return false;

    result.clear();

    std::string sql;
    if (isDigitString(info)) {
        sql = "SELECT * FROM Patient_Info WHERE patient_id_diy = ?";
    } else {
        sql = "SELECT * FROM Patient_Info WHERE patient_name = ?";
    }

    sqlite3_stmt* stmt;
    const int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || !stmt) {
        qDebug() << "[SQLite] prepare failed:" << sqlite3_errmsg(db);
        return false;
    }
    sqlite3_bind_text(stmt, 1, info.c_str(), -1, SQLITE_TRANSIENT);

    const int colCount = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        for (int i = 0; i < colCount; ++i) {
            const char* key = sqlite3_column_name(stmt, i);
            const auto val =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[key] = val ? val : "";
        }
        result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return !result.empty();
}

// todo
bool PatientDatabase::searchPatient(const std::string& info, std::string& accountInfo, std::string& idInfo) {
    if (!db) return false;
    accountInfo.clear();
    idInfo.clear();

    std::vector<Row> result;
    const bool found = searchPatient(info, result);

    if (!found || result.size() != 1)
        return false;
    const Row& row = result.front();

    auto itAccount = row.find("patient_name");   // 或 account
    auto itId      = row.find("patient_id_diy");  // 或 id

    if (itAccount == row.end() || itId == row.end())
        return false;

    accountInfo = itAccount->second;
    idInfo      = itId->second;

    return true;

}

// DANGER: 数据库的规范使用
// bug 不出现了 暂且归为 数据库文件不能同时使用(无法复现) & operator关键字(同样无法复现)
// throw std::runtime_error("Cannot open database"); 这行导致bug
/*
PatientDatabase::PatientDatabase(const std::string& dbPath)
{
    db = nullptr;

    try {
        const int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string err = db ? sqlite3_errmsg(db) : "unknown error";
            if (db) {
                sqlite3_close(db);
                db = nullptr;
            }
            throw std::runtime_error("Cannot open database: " + err);
        }
    }
    catch (const std::exception& e) {
        qCritical() << "[SQLite]" << e.what();
        // 你可以选择：
        // 1. 继续抛出
        // throw;
        // 2. 或仅记录错误，让对象处于“不可用状态”
    }
}
*/
bool PatientDatabase::addPatient(
    const int patient_id_diy,
    const std::string& patient_name,
    const std::string& gender,
    const int age,
    const double height_cm,
    const double weight_kg,
    const double blood_pressure_sys,
    const double blood_pressure_dia,
    const std::string& clinical_diagnosis,
    const double invasive_icp,
    const std::string& operator_name,
    const int operator_ID)
{
    if (!db) return false;
    std::vector<Row> rows;
    const bool exists = searchPatient(std::to_string(patient_id_diy), rows);

    std::string sql;
    if (exists) {
        sql =
            "UPDATE Patient_Info SET "
            "patient_name=?, gender=?, age=?, height_cm=?, weight_kg=?, "
            "blood_pressure_sys=?, blood_pressure_dia=?, clinical_diagnosis=?, "
            "invasive_icp=?, operator_name=?, operator_ID=?, update_time=? "
            "WHERE patient_id_diy=?";
    } else {
        sql =
            "INSERT INTO Patient_Info "
            "(patient_id_diy, patient_name, gender, age, height_cm, weight_kg,"
            "blood_pressure_sys, blood_pressure_dia, clinical_diagnosis,"
            "invasive_icp, operator_name, operator_ID) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || !stmt) {
        qDebug() << "[SQLite] prepare failed:" << sqlite3_errmsg(db);
        return false;
    }

    int idx = 1;
    if (!exists)
        sqlite3_bind_int(stmt, idx++, patient_id_diy);

    sqlite3_bind_text(stmt, idx++, patient_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, idx++, age);
    sqlite3_bind_double(stmt, idx++, height_cm);
    sqlite3_bind_double(stmt, idx++, weight_kg);
    sqlite3_bind_double(stmt, idx++, blood_pressure_sys);
    sqlite3_bind_double(stmt, idx++, blood_pressure_dia);
    sqlite3_bind_text(stmt, idx++, clinical_diagnosis.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, idx++, invasive_icp);
    sqlite3_bind_text(stmt, idx++, operator_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, idx++, operator_ID);

    if (exists) {
        const std::string timeStr = nowTime();
        sqlite3_bind_text(stmt, idx++, timeStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, idx++, patient_id_diy);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qDebug() << "[SQLite] step failed:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    if (true) {
        std::cout<<true<<std::endl;
    }

    return true;
}

// if (!exists && ok) {
//     // 新建patient路径
//     // std::filesystem::create_directories(
//     //     appPath + "/each_P_data/" +
//     //     patient_name + "_" + std::to_string(patient_id_diy)
//     // );
// }

int PatientDatabase::getNextPatientID() {
    const char* sql =
        "SELECT seq FROM sqlite_sequence WHERE name='Patient_Info'";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    int nextID = 1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        nextID = sqlite3_column_int(stmt, 0) + 1;
    }

    sqlite3_finalize(stmt);
    return nextID;
}

EachPatientDatabase::EachPatientDatabase(const std::string &appPath) : appPath(appPath) {
    const std::string dbPath = this->appPath + "/each_P_data/count_patient_test.db";
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database");
    }
}

void EachPatientDatabase::setANDget_info(const std::string &_name, const std::string &_id, QVector<QStringList>& infos) {
    name = _name;
    id = _id;
    infos.clear();
    const QString sql = QString("SELECT test_id, test_time_start, test_time_duration FROM %1_%2").arg(name, id);

    sqlite3_stmt* stmt = nullptr;
    const int rc = sqlite3_prepare_v2(db, sql.toUtf8().constData(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK || !stmt) {
        qDebug() << "[SQLite] prepare failed:" << sqlite3_errmsg(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QStringList row;

        const char* col0 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* col1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* col2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        row << (col0 ? col0 : "")
            << (col1 ? col1 : "")
            << (col2 ? col2 : "");

        infos.push_back(row);
    }

    sqlite3_finalize(stmt);
}
