#include "database/database.hpp"

// database connection's data
const std::string host = "tcp://127.0.0.1:3306";
const std::string user = "root";
const std::string password = "1972";
const std::string database = "osint";

int main()
{
    Database db;

    if (!db.connect(host, user, password, database))
    {
        std::cout << "Connection faild\n";
        return 0;
    }


    auto users = db.GetUsers();

    for (const auto &usr : users)
    {
        std::cout << usr.id << '\n';
        std::cout << usr.username << '\n';
        std::cout << usr.normalized << '\n';
    }

    db.disconnect();

    return 0;
}