//
// Created by Administrator on 26-1-13.
//

#ifndef PATIENT_DATABASE_H
#define PATIENT_DATABASE_H

#pragma once
#include <QWidget>
#include <string>
#include <vector>
#include <map>
#include <variant>

#include "../../third_src/sqlite3/sqlite3.h"

class PatientDatabase {
public:
    explicit PatientDatabase(std::string appPath);
    ~PatientDatabase();

    void close();

    bool searchPatient(const std::string& info, std::vector<std::map<std::string, std::string>>& result);
    bool searchPatient(const std::string& info, std::string& accountInfo, std::string& idInfo);

    bool addPatient(
        int patient_id_diy,
        const std::string& patient_name,
        const std::string& gender,
        int age,
        double height_cm,
        double weight_kg,
        double blood_pressure_sys,
        double blood_pressure_dia,
        const std::string& clinical_diagnosis,
        double invasive_icp,
        const std::string& operator_name,
        int operator_ID
    );

    int getNextPatientID();

private:
    sqlite3* db{};
    std::string appPath;

    bool isDigitString(const std::string& s);
    std::string nowTime();
};

class EachPatientDatabase {
public:
    explicit EachPatientDatabase(const std::string& appPath);
    // ~EachPatientDatabase();

    void setANDget_info(const std::string& _name, const std::string& _id, QVector<QStringList>& infos);

    void close();

private:
    sqlite3* db = nullptr;

    std::string appPath = "";

public:
    std::string name = "";
    std::string id = "";

};
#endif //PATIENT_DATABASE_H
