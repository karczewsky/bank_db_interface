//
// Created by jakub on 11/12/18.
//

#ifndef BANK_CLIENT_H
#define BANK_CLIENT_H

#include <string>

#include "Account.h"

using namespace std;

enum class Gender {
    undefined = 0,
    male = 1,
    female = 2
};

class Client {
private:
    unsigned int client_id;
    string first_name;
    string last_name;
    Gender gender;
    string city;
    string street;
    string postal_code;
    string email;
    void enter_address_data();
    void enter_email_data();
    void delete_from_db();
    void push_update();
    void get_summed_accounts_cash();
    friend class Account;
public:
    Client();
    explicit Client(unsigned int client_id);
    void print();
    void print_sub_accounts();
    static void print_all();
    static void personal_submenu();
};


#endif //BANK_CLIENT_H
