//
// Created by jakub on 11/12/18.
//

#ifndef BANK_DB_H
#define BANK_DB_H

#include <string>
#include <pqxx/pqxx>

using namespace std;

typedef pqxx::work transaction;
typedef pqxx::result result;

class Db {
public:
    static string connection_string;
    static pqxx::connection connection;
};


#endif //BANK_DB_H
