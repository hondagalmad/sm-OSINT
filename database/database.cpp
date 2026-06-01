#include "database.hpp"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

Database::Database()
{
    driver = nullptr;
}

bool Database::connect(const std::string &host, const std::string &user, const std::string &password, const std::string &database)
{
    try
    {
        driver = sql::mysql::get_mysql_driver_instance();
        con.reset(
            driver->connect(host, user, password));
        con->setSchema(database);

        return true;
    }
    catch (sql::SQLException &e)
    {
        return false;
    }
}

void Database::disconnect()
{
    con.reset();
}
