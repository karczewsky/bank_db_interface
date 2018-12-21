//
// Created by jakub on 11/12/18.
//

#include <pqxx/pqxx>
#include <iostream>
#include <iomanip>

#include "Client.h"
#include "Db.h"


string parseGenderValue(int i) {
    switch ((Gender)i) {
        case Gender::female:
            return "Kobieta";
        case Gender::male:
            return "Mezczyzna";
        case Gender::undefined:
            return "Inna";
        default:
            return "Err";
    }
}


Client::Client() {
    cout << "Podaj imie klienta:" << endl;
    getline(cin, this->first_name);
    cout << "Podaj nazwisko klienta:" << endl;
    getline(cin, this->last_name);
    while(true){
        cout << "Podaj plec(1.Mezczyzna 2.Kobieta 0.Inna):" << endl;
        int p; cin >> p;
        cin.ignore();
        if (p >= 0 && p <= 2) {
            this->gender = Gender(p);
            break;
        } else {
            cout << "Podaj prawidlowa wartosc plci." << endl;
        }
    }
    this->enter_address_data();
    this->enter_email_data();

    transaction txn(Db::connection);
    result res;

    res = txn.exec(
            "INSERT INTO client(first_name, last_name, gender, city, street, postal_code, email) "
            "VALUES("+txn.quote(this->first_name) + ", " + txn.quote(this->last_name) + ", " +
            txn.quote((int)this->gender) + ", " + txn.quote(this->city) + ',' + txn.quote(this->street) + ", " +
            txn.quote(this->postal_code) + ", NULLIF(" + txn.quote(this->email) + ",\'\')) RETURNING client_id"
    );

    txn.commit();
    this->client_id = res[0][0].as<unsigned int>();
}


Client::Client(unsigned int client_id) {
    transaction txn(Db::connection);

    result res = txn.exec("SELECT * FROM Client WHERE client_id=" + txn.quote(client_id));

    if(res.size() != 1)
        throw "Not found in database";

    this->client_id = res[0][0].as<unsigned int>();
    this->first_name = res[0][1].as<string>();
    this->last_name = res[0][2].as<string>();
    this->gender = Gender(res[0][3].as<int>());
    this->city = res[0][4].as<string>();
    this->street = res[0][5].as<string>();
    this->postal_code = res[0][6].as<string>();
    this->email = (res[0][7].is_null()) ? "N/A" : res[0][7].as<string>();
}

void Client::print() {
    cout << endl;
    cout << "==============================" << endl;
    cout << "Client_id: " << this->client_id << endl;
    cout << "Name: " << this->first_name << " " << this->last_name << endl;
    string e = (this->email.empty()) ? "N/A" : this->email;
    cout << "Email: " << e << endl;
    cout << "Address: " << this->street << ", " << this->city << " " << this->postal_code << endl;
    cout << "==============================" << endl;
}

void Client::delete_from_db() {
    transaction txn(Db::connection);
    result res = txn.exec("DELETE FROM client where client_id=" + txn.quote(this->client_id) + " RETURNING client_id");
    if(res[0][0].as<int>() != this->client_id) {
        cout << "BLAD: Klient nie zostal usuniety z bazy!" << endl;
    } else {
        cout << "Klient " << this->first_name << " " << this->last_name
        << "(ID=" << this->client_id << ") zostal usuniety z bazy." << endl;
        txn.commit();
    }
}

void Client::push_update() {
    transaction txn(Db::connection);
    txn.exec("UPDATE client SET first_name = " +txn.quote(this->first_name) +
            " ,last_name = " + txn.quote(this->last_name) + " ,gender = " + txn.quote(int(this->gender)) +
            " ,city = " + txn.quote(this->city) + " ,street = " + txn.quote(this->street) +
            " ,postal_code = " + txn.quote(this->postal_code) + " ,email = " + txn.quote(this->email) +
            "WHERE client_id = " + txn.quote(this->client_id));
    txn.commit();
}

void Client::enter_address_data() {
    cout << "Podaj dane adresowe:" << endl;
    cout << "Miasto: " << endl;
    getline(cin, this->city);
    cout << "Ulice: " << endl;
    getline(cin, this->street);
    while (true) {
        cout << "Kod pocztowy: " << endl;
        string k;
        getline(cin, k);
        if (k.length() == 6) {
            this->postal_code = k;
            break;
        } else {
            cout << "Podaj prawidlowy kod pocztowy." << endl;
        }
    }
}

void Client::enter_email_data() {
    cout << "Podaj e-mail: " << endl;
    getline(cin, this->email);
}


void Client::print_sub_accounts() {
    transaction txn(Db::connection);
    result res = txn.exec("SELECT account_id, money, max_debit FROM account "
                                "WHERE client_id = " + txn.quote(this->client_id));

    cout << "Konta klienta " << this->last_name << " " << this->first_name << ":" << endl;
    cout << left << setfill(' ') << setw(10) <<  "Id_konta";
    cout << left << setfill(' ') << setw(18) << "Max Debet - PLN";
    cout << left << setfill(' ') << setw(20) << "Oszczednosci - PLN";
    cout << endl;
    int summed = 0;
    for (auto row = res.begin(); row != res.end(); row++) {
        cout << left << setfill(' ') << setw(10) <<  row[0].c_str();
        cout << left << setfill(' ') << setw(18) << row[2].as<int>()/100;
        cout << left << setfill(' ') << setw(20) << Account::format_cash(row[1].as<int>());
        cout << endl;

        summed += row[1].as<int>();
    }
    cout << left << setfill(' ') << setw(10) <<  "SUMA";
    cout << left << setfill(' ') << setw(18) << " ";
    cout << left << setw(20) << Account::format_cash(summed);
    cout << endl;
}


void Client::print_all() {
    transaction txn(Db::connection);
    result res = txn.exec("SELECT * FROM Client");

    if (res.empty()) {
        cout << "No clients in DB!" << endl;
    } else {
        cout << left << setfill(' ') << setw(4) << "Id";
        cout << left << setfill(' ') << setw(10) << "Imie";
        cout << left << setfill(' ') << setw(17) << "Nazwisko";
        cout << left << setfill(' ') << setw(13) << "Plec";
        cout << left << setfill(' ') << setw(13) << "Miasto";
        cout << left << setfill(' ') << setw(20) << "Ulica";
        cout << left << setfill(' ') << setw(13) << "Kod pocztowy";
        cout << left << setfill(' ') << setw(13) << "E-mail";
        cout << endl;
        for(auto row = res.begin(); row != res.end(); row++) {
            cout << left << setfill(' ') << setw(4) << row[0].c_str();
            cout << left << setfill(' ') << setw(10) << row[1].c_str();
            cout << left << setfill(' ') << setw(17) << row[2].c_str();
            cout << left << setfill(' ') << setw(13) << parseGenderValue(row[3].as<int>());
            cout << left << setfill(' ') << setw(13) << row[4].c_str();
            cout << left << setfill(' ') << setw(20) << row[5].c_str();
            cout << left << setfill(' ') << setw(13) << row[6].c_str();
            string e = (row[7].is_null()) ? "N/A" : row[7].c_str();
            cout << left << setfill(' ') << setw(13) << e;
            cout << endl;
        }
    }
}


void Client::personal_submenu() {
    cout << "Podaj id klienta do zalogowania: ";
    int id; cin >> id;
    cin.ignore();
    shared_ptr<Client> c;
    try {
        c.reset(new Client(id));
    } catch (...) {
        cout << "Blad logowania klienta! Powrot do glownego menu!" << endl;
        return;
    }

    bool menu = true;
    int option;
    while (menu) {
        cout << "=========== KLIENT MENU ==========" << endl;
        cout << "\t0) Wyjdz do poprzedniego menu" << endl;
        cout << "\t1) Wypisz informacje o mnie" << endl;
        cout << "\t2) Zaktualizuj email" << endl;
        cout << "\t3) Zaktualizuj adres zamieszkania" << endl;
        cout << "\t4) Stworz nowe konto bankowe" << endl;
        cout << "\t5) Usun konto klienckie" << endl;
        cout << "\t6) Wypisz konta bankowe zwiazane z klientem" << endl;
        cout << "\t7) Przejdz do obslugi konta" << endl;
        cout << "\t8) Wypisz dostepne srodki na wszystkich kontach" << endl;
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
                c->print();
                break;
            }
            case 2: {
                c->enter_email_data();
                c->push_update();
                cout << "Nowy email zostal zapisany" << endl;
                break;
            }
            case 3: {
                c->enter_address_data();
                c->push_update();
                cout << "Nowy adres zostal zapisany" << endl;
                break;
            }
            case 4: {
                Account acc = Account(*c);
                cout << "Dodano poniższe konto:" << endl;
                acc.print();
                break;
            }
            case 5: {
                c->delete_from_db();
                cout << "Konto zostalo usuniete. Wylogowywanie." << endl;
                menu = false;
                break;
            }
            case 6: {
                c->print_sub_accounts();
                break;
            }
            case 7: {
                Account::account_submenu(*c);
                break;
            }
            case 8: {
                c->get_summed_accounts_cash();
                break;
            }
        }
    }
}

void Client::get_summed_accounts_cash() {
    auto accounts = Account::get_client_accounts(*this);
    int summed = 0;
    for(auto& a : accounts) {
        summed += get_friendly_money(a);
    }
    cout << "Obecnie na kontach przechowywane jest: " + Account::format_cash(summed) << " PLN." << endl ;
    return;
}