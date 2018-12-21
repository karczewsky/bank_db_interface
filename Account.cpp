//
// Created by jakub on 14/12/18.
//

#include <iostream>
#include <pqxx/pqxx>
#include <iomanip>
#include <algorithm>

#include "Account.h"
#include "Db.h"

// Creating new account for existing client
Account::Account(Client &c) : client(c) {
    transaction txn(Db::connection);
    result res = txn.exec("INSERT INTO account(client_id) VALUES (" + txn.quote(this->client.client_id) + ")"
            + "RETURNING account_id, money, max_debit");
    txn.commit();
    this->acc_id = res[0][0].as<unsigned int>();
    this->money = res[0][1].as<int>();
    this->max_debit = res[0][2].as<int>();
}

Account::Account(Client &c,  unsigned int account_id) : client(c){
    transaction txn(Db::connection);
    result res = txn.exec("SELECT money, max_debit FROM account "
                                "WHERE account_id=" + txn.quote(account_id) +
                                "AND client_id=" + txn.quote(c.client_id));
    if (res.size() != 1) {
        throw "Unauthorized to see account";
    }

    this->acc_id = account_id;
    this->money = res[0][0].as<int>();
    this->max_debit = res[0][1].as<int>();
}


void Account::print() {
    cout << "==============================" << endl;
    cout << "Konto nr: " << this->acc_id << endl;
    cout << "Wlasciciel: " << this->client.last_name << " " << this->client.first_name
    << "(Client_ID=" << this->client.client_id << ")" << endl;
    cout << "Oszczednosci: " << Account::format_cash(this->money) << " PLN" << endl;
    cout << "Ustawiony maksymalny debet: " << Account::format_cash(this->max_debit) << " PLN" << endl;
    cout << "==============================" << endl;
}


void Account::setDebit() {
    cout << "Podaj nowa wartosc debetu(PLN): ";
    int new_debit; cin >> new_debit;
    while(new_debit < 0) {
        cout << "Debet nie moze byc ujemny, podaj poprawna wartosc: ";
        cin >> new_debit;
    }
    cin.ignore();
    new_debit *= 100;

    if (this->money > 0 || new_debit >= -this->money) {
        transaction txn(Db::connection);
        txn.exec("UPDATE account SET max_debit = " + txn.quote(new_debit) +
        " WHERE account_id = " + txn.quote(this->acc_id));
        txn.commit();
        this->max_debit = new_debit;

        cout << "Debet poprawnie zmieniony." << endl;
    } else {
        cout << "Debet mniejszy od obecnego zadluzenia! Debet nie zostal zmieniony!" << endl;
    }
}

void Account::payment(int p, string description) {
    transaction txn(Db::connection);

    if(p < 0) {
        result res = txn.exec("SELECT money + max_debit FROM account WHERE account_id = " + txn.quote(this->acc_id));
        int max_payment = res[0][0].as<int>();
        if (p < -max_payment) {
            throw "Not enought money/debit";
        }
    }

    txn.exec("UPDATE account SET money = money + " + txn.quote(p) +
             " WHERE account_id = " + txn.quote(this->acc_id));

    txn.exec("INSERT INTO transaction(account_id, description, amount) "
             "VALUES(" + txn.quote(this->acc_id) + ", NULLIF(" +txn.quote(description) + ", \'\'), " + txn.quote(p) + ")");

    this->money += p;

    txn.commit();
}


void Account::deposit() {
    cout << "Ile chcesz wplacic: ";
    int d = Account::read_decimal_as_int();
    while (d <= 0) {
        cout << "Prosze podac poprawna wartosc: ";
        d = Account::read_decimal_as_int();
    }

    this->payment(d, "Wplata na konto");
    cout << "Poprawnie wplacono " << Account::format_cash(d) << " PLN na konto." << endl;
}


void Account::withdraw() {
    cout << "Ile chcesz wyplacic: ";
    int d = Account::read_decimal_as_int();
    while (d <= 0) {
        cout << "Prosze podac poprawna wartosc: ";
        d = Account::read_decimal_as_int();
    }

    try {
        this->payment(-d, "Wyplata z konta");
        cout << "Poprawnie wyplacono " << Account::format_cash(d) << " PLN z konta." << endl;
    } catch (...) {
        cout << "Niewystarczajaca ilosc pieniedzy na koncie." << endl;
    }
}

void Account::transfer() {
    cout << "Podaj ile chcesz przelac: ";
    int d = Account::read_decimal_as_int();
    while (d <= 0) {
        cout << "Prosze podac poprawna wartosc: ";
        d = Account::read_decimal_as_int();
    }

    transaction txn(Db::connection);
    result res = txn.exec("SELECT max_debit + money FROM account WHERE account_id = " + txn.quote(this->acc_id));
    int max = res[0][0].as<int>();
    if (d > max) {
        cout << "Za malo srodkow na koncie do wykonania takiego przelewu." << endl;
        cout << "Dostepne srodki: " << Account::format_cash(max) << " PLN" << endl;
        return;
    }

    cout << "Podaj docelow nr konta: " << endl;
    int dest_acc_num; cin >> dest_acc_num;
    cin.ignore();

    res = txn.exec("SELECT count(*) from account WHERE account_id = " + txn.quote(dest_acc_num));

    if(res[0][0].as<int>() != 1) {
        cout << "Bledne docelowe konto." << endl;
        return;
    }

    txn.exec("UPDATE account SET money = money + " + txn.quote(d) + " WHERE account_id = " + txn.quote(dest_acc_num));
    txn.exec("UPDATE account SET money = money - " + txn.quote(d) + " WHERE account_id = " + txn.quote(this->acc_id));
    txn.exec("INSERT INTO transaction(account_id, description, amount) "
             "VALUES(" + txn.quote(dest_acc_num) + ", " +
             txn.quote("Przelew przychodzacy od: " + to_string(this->acc_id)) + ", " + txn.quote(d) + ")");
    txn.exec("INSERT INTO transaction(account_id, description, amount) "
             "VALUES(" + txn.quote(this->acc_id) + ", " +
             txn.quote("Przelew wychodzacy do: " + to_string(dest_acc_num)) + ", " + txn.quote(d) + ")");
    txn.commit();

    cout << "Przelew zostal wykonany." << endl;
}


void Account::delete_from_db() {
    transaction txn(Db::connection);
    result res = txn.exec("DELETE from account WHERE account_id = " + txn.quote(this->acc_id) + " RETURNING account_id, money");
    if(res[0][0].as<int>() != this->acc_id) {
        cout << "BLAD: Klient nie zostal usuniety z bazy!" << endl;
    } else if(res[0][1].as<int>() != 0) {
        cout << "BLAD: Konto nie moze byc usuniete, poniewaz ma niezerowa ilosc pieniedzy na nim." << endl;
        cout << "Prosze je wyplacic/przelac na inne konto. W przypadku zadluzen musza one zostac uregulowane." << endl;
    } else {
        txn.commit();
    }
}

void Account::print_transaction_history() {
    transaction txn(Db::connection);
    result res = txn.exec("SELECT date, amount, description FROM transaction"
                               " WHERE account_id = " + txn.quote(this->acc_id));

    cout << left << setfill(' ') << setw(27) << "Data";
    cout << left << setfill(' ') << setw(13) << "Ilosc(PLN)";
    cout << left << setfill(' ') << setw(20) << "Opis";
    cout << endl;
    for (auto row = res.begin(); row != res.end(); row++) {
        cout << left << setfill(' ') << setw(27) << row[0].c_str();
        cout << left << setfill(' ') << setw(13) << Account::format_cash(row[1].as<int>());
        cout << left << setfill(' ') << setw(20) << row[2].c_str();
        cout << endl;
    }
}

string Account::format_cash(int num) {
    bool negative = num < 0;
    num = num < 0 ? -num : num;
    string a = std::to_string(num / 100);
    int x = num % 100;
    string b = std::to_string(x);
    b = b.length() == 1 ? "0"+b : b;
    a = negative ? "-" + a : a;
    return a + "." + b;
}

int Account::read_decimal_as_int() {
    string line;
    getline(cin, line);
    size_t pos = line.find('.');
    if (pos != string::npos) {
        // found '.'
        string first = line.substr(0, pos);
        string second = line.substr(pos+1);
        try {
            second = (second.length() == 1) ? second + "0" : second;
            return stoi(first) * 100 + stoi(second);
        } catch (...) {
            return -1;
        }
    } else {
        // notfound '.'
        try {
            return stoi(line) * 100;
        } catch (...) {
            return -1;
        }
    }
}


void Account::account_submenu(Client &c) {
    cout << "Podaj id konta do zalogowania: ";
    unsigned int id; cin >> id;
    cin.ignore();
    shared_ptr<Account> a;
    try {
        a.reset(new Account(c, id));
    } catch (...) {
        cout << "Blad logowania do konta!" << endl;
        return;
    }

    bool menu = true;
    int option;
    while (menu) {
        cout << "=========== KONTO MENU ==========" << endl;
        cout << "\t0) Wyjdz do poprzedniego menu" << endl;
        cout << "\t1) Wypisz informacje o koncie" << endl;
        cout << "\t2) Ustaw nowy debet" << endl;
        cout << "\t3) Dokonaj wplaty na konto" << endl;
        cout << "\t4) Dokonaj wyplaty z konta" << endl;
        cout << "\t5) Wykonaj przelew" << endl;
        cout << "\t6) Usun konto" << endl;
        cout << "\t7) Wypisz historie transakcji" << endl;
        cout << "Wybierz opcje: ";

        cin >> option;
        cin.ignore();

        switch (option) {
            default: {
                cout << "Bledna opcja" << endl;
                break;
            }
            case 0: {
                menu = false;
                break;
            }
            case 1: {
                a->print();
                break;
            }
            case 2: {
                a->setDebit();
                break;
            }
            case 3: {
                a->deposit();
                break;
            }
            case 4: {
                cout << "Obecnie na koncie: " << Account::format_cash(a->money) << endl;
                cout << "Obecny maksymalny debet: " << Account::format_cash(a->max_debit) << endl;
                a->withdraw();
                break;
            }
            case 5: {
                a->transfer();
                break;
            }
            case 6: {
                a->delete_from_db();
                break;
            }
            case 7: {
                a->print_transaction_history();
                break;
            }
        }
    }
}

int get_friendly_money(Account &a) {
    return a.money;
};

vector<Account> Account::get_client_accounts(Client &c) {
    transaction txn(Db::connection);
    result res = txn.exec("SELECT account_id FROM account "
                                "WHERE client_id=" + txn.quote(c.client_id));

    txn.abort();
    if (res.size() < 1) {
        throw "Unauthorized to see account";
    }
    vector<Account> list;

    for(auto row = res.begin(); row != res.end(); row++) {
        unsigned int account_id = row[0].as<unsigned int>();
        list.push_back(Account(c, account_id));
    }

    return list;
}
