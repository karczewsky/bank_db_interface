//
// Created by jakub on 14/12/18.
//

#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#include "Client.h"

using namespace std;

class Client;

class Account {
private:
    unsigned int acc_id;
    Client &client;
    int money;
    int max_debit;
    Account(Client &c, unsigned int account_id);
    void setDebit();
    void deposit();
    void withdraw();
    void payment(int p, string description);
    void transfer();
    void delete_from_db();
    void print_transaction_history();
    friend int get_friendly_money(Account &a);
public:
    Account(Client &c);
    void print();
    static string format_cash(int num);
    static void account_submenu(Client &c);
    static int read_decimal_as_int();
    static vector<Account> get_client_accounts(Client &c);
};


int get_friendly_money(Account &a);


#endif //BANK_ACCOUNT_H
