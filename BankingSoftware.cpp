/**
*
* Solution to course project # 9
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2022/2023
*
* @author Monika Angelova
* @idnumber 1MI0600263
* @compiler VC
*
* <main file for project>
*
*/

#include <iostream>
#include <iomanip>
#include <optional>
#include <regex>
#include <map>
#include <limits>
#include <cmath>
#include <fstream>
using namespace std;

typedef string Username;
typedef int PasswordHash;
typedef unsigned long MoneyAmount;
typedef long long MoneyBalance;
const MoneyBalance OVERDRAFT_LIMIT = -10'000'00;

struct User {
    Username username;
    PasswordHash passwordHash;
    MoneyBalance rawBalance = 0;

    void deposit(MoneyAmount amount) {
        if (rawBalance > 0 && rawBalance + amount < 0) {
              cout << "Amount is too large" <<   endl;
            return;
        }
        rawBalance += amount;
    }

    bool withdraw(MoneyAmount amount) {
        MoneyBalance withdraw = (MoneyBalance)amount;
        if (rawBalance - withdraw < OVERDRAFT_LIMIT) {
            return false;
        }
        rawBalance -= withdraw;
        return true;
    }

    float balance() const {
        return (float)rawBalance / 100;
    }

};

struct Bank {
      map<Username, User> users;
      optional<User> loggedInUser;

      optional<User> login(const Username& username,
        const PasswordHash& passwordHash) {
        auto search = users.find(username);
        if (search == users.end() || search->second.passwordHash != passwordHash) {
            return   nullopt;
        }

        loggedInUser = search->second;
        users.erase(search);
        return loggedInUser;
    }

    void cancelAccount(const PasswordHash& passwordHash) {
        if (!loggedInUser || loggedInUser->passwordHash != passwordHash || loggedInUser->balance() != 0) {
            return;
        }

        loggedInUser =   nullopt;
    }

    bool transfer(MoneyAmount amount, const Username& recipient) {
        auto toUser = users.find(recipient);
        if (toUser == users.end()) {
            return false;
        }
        if (!loggedInUser || !loggedInUser->withdraw(amount)) {
            return false;
        }
        toUser->second.deposit(amount);
        return true;
    }

    void logout() {
        if (!loggedInUser) {
            return;
        }
        users[loggedInUser->username] = *loggedInUser;
        loggedInUser =   nullopt;
    }

    void newUser(const User& user) {
        loggedInUser = user;
    }
};

bool isValidPassword(const   string& input) {
    if (input.size() < 5) {
          cout << "This password is too short. It must be at least 5 characters." <<   endl;
        return false;
    }

      regex lowerCase =   regex("[a-z]+");
    if (!  regex_search(input, lowerCase)) {
          cout << "This password does not contain a lower case letter." <<   endl;
        return false;
    }
      regex upperCase =   regex("[A-Z]+");
    if (!  regex_search(input, upperCase)) {
          cout << "This password does not contain an upper case letter." <<   endl;
        return false;
    }

      regex symbol =   regex("[!@#$%^&*]+");
    if (!  regex_search(input, symbol)) {
          cout << "This password does not contain a symbol" <<   endl;
        return false;
    }

    return true;
}

int insecureHash(const   string& input) {
    int hash = 0;
    for (int i = 0; i < input.length(); ++i) {
        hash += input[i] *   pow(31, i);
    }
    return hash;
}

  optional<PasswordHash> inputPassword(bool validate = false) {
      cout << "Password:";
      string passwordInput;
      getline(  cin, passwordInput);
    if (validate) {
        if (!isValidPassword(passwordInput)) {
            return   nullopt;
        }

          cout << "Confirm password:";
          string confirmInput;
          getline(  cin, confirmInput);
        if (passwordInput != confirmInput) {
              cout << "The two passwords do not match." <<   endl;
            return   nullopt;
        }
    }

    return insecureHash(passwordInput);
}

MoneyAmount inputAmount() {
      cout << "Amount:";
      string amountInput;
      getline(  cin, amountInput);
    double amount =   stod(amountInput);
    if (amount <= 0) {
          cout << "Amount must be positive." <<   endl;
        return 0;
    }
    // keep 2 decimal places and convert to integer
    amount *= 100;
    // overflow check
    if (amount >   numeric_limits<MoneyAmount>::max()) {
          cout << "Amount is too large." <<   endl;
        return 0;
    }
    return (MoneyAmount)amount;
}

void commandsMenu(Bank& bank) {
    while (bank.loggedInUser) {
          cout <<
            "You have " << bank.loggedInUser->balance() << " BGN. " <<
            "Choose one of the following options:" <<   endl <<
            "C - cancel account" <<   endl <<
            "D - deposit" <<   endl <<
            "L - logout" <<   endl <<
            "T - transfer" <<   endl <<
            "W - withdraw" <<   endl;

          string input;
          getline(  cin, input);

        if (input == "C") {
            bank.cancelAccount(*inputPassword());
        }
        else if (input == "D") {
            bank.loggedInUser->deposit(inputAmount());
        }
        else if (input == "L") {
            bank.logout();
        }
        else if (input == "T") {
              cout << "Recipient username:";
              string recipient;
              getline(  cin, recipient);
            bank.transfer(inputAmount(), recipient);
        }
        else if (input == "W") {
            bank.loggedInUser->withdraw(inputAmount());
        }
    }
}

bool isValidUsername(const   string& input, const   map<  string, User>& users) {
      regex containsValidChar =   regex("^[A-Za-z!@#\\$%\\^&\\*]+$");

    if (!  regex_match(input, containsValidChar)) {
          cout << "This username is invalid. Use only letters A-Z, a-z or symbols !@#$%^&*" <<   endl;
        return false;
    }

    if (auto search = users.find(input); search != users.end()) {
          cout << "This username is already taken." <<   endl;
        return false;
    }

    return true;
}

void registerMenu(Bank& bank) {
      cout << "Register a new user by filling the prompts:" <<   endl;

      cout << "Username:";
      string usernameInput;
      getline(  cin, usernameInput);
    if (!isValidUsername(usernameInput, bank.users)) {
        return;
    }

    auto passwordHash = inputPassword(true);
    if (!passwordHash) {
        return;
    }

    bank.newUser({ usernameInput, *passwordHash });
    commandsMenu(bank);
}

void loginMenu(Bank& bank) {
      cout << "Username:";
      string usernameInput;
      getline(  cin, usernameInput);
    auto user = bank.login(usernameInput, *inputPassword());
    if (!user) {
          cout << "Failed login." <<   endl;
        return;
    }
    commandsMenu(bank);
}

// return true if save is successful, false otherwise
bool saveUsersFile(const Bank& bank) {
      ofstream output("users.txt",   ios::trunc);
    if (!output.is_open()) {
          cout << "Failed to close file users.txt" <<   endl;
        return false;
    }
    for (auto it = bank.users.begin(); it != bank.users.end(); ++it) {
        auto user = it->second;
        output << user.username << ":" << user.passwordHash << ":" << user.rawBalance <<   endl;
    }
    output.close();
    return true;
}

void mainMenu(Bank& bank) {
    bool quit = false;
    while (!quit) {
          cout <<
            "Choose one of the options:" <<   endl <<
            "L - login" <<   endl <<
            "R - register" <<   endl <<
            "Q - save and quit" <<   endl;

          string input;
          getline(  cin, input);

        if (input == "L") {
            loginMenu(bank);
        }
        else if (input == "R") {
            registerMenu(bank);
        }
        else if (input == "Q") {
            // quit if save succeeds
            quit = saveUsersFile(bank);
        }
    }
}

void initOutputFormatting() {
    // output money with fixed 2 decimal places
      cout <<   fixed <<   setprecision(2);
}

  optional<Bank> bankFromUsersFile() {
      ifstream input("users.txt",  ios::in);
    if (!input.is_open()) {
          cout << "Failed to open file users.txt" <<   endl;
        return   nullopt;
    }
      map<Username, User> users = {};
    for (  string username;   getline(input, username, ':');) {
          string passwordHashIn;
          getline(input, passwordHashIn, ':');
          string balanceIn;
          getline(input, balanceIn);
        PasswordHash passwordHash =   stoi(passwordHashIn);
        MoneyBalance balance =   stoll(balanceIn);
        User user = { username, passwordHash, balance };
        users[username] = user;
    }
    input.close();
    return Bank{ users };
}

int main() {
    initOutputFormatting();
    auto bank = bankFromUsersFile();
    if (!bank) {
          cout << "Press [Enter] to close the program." <<   endl;
          string input;
          getline(  cin, input);
        return 0;
    }
    mainMenu(*bank);
    return 0;
}