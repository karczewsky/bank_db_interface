#include <iostream>
#include <pqxx/pqxx>

#include "Client.h"

using namespace std;


int main() {
    cout << "Witaj w systemie bankowym." << endl;
    int choice;
    bool menu_loop = true;
    while (menu_loop) {
        cout << "============== MENU ==============" << endl;
        cout << "\t0) Wyjdz z systemu" << endl;
        cout << "\t1) Wypisz wszystkich klientow" << endl;
        cout << "\t2) Dodaj nowego klienta" << endl;
        cout << "\t3) Przejdz do submenu klienta" << endl;
        cout << "Wybierz opcje: ";

        cin >> choice;
        cin.ignore();
        switch(choice) {
            default: {
                cout << "Bledna opcja!" << endl;
                break;
            }
            case 0: {
                menu_loop = false;
                break;
            }
            case 1: {
                Client::print_all();
                break;
            }
            case 2: {
                Client c = Client();
                cout << "Dodano ponizszego klienta." << endl;
                c.print();
                cout << endl;
                break;
            }
            case 3: {
                Client::personal_submenu();
                break;
            }
        }
    }
    return 0;
}