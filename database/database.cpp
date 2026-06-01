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

bool Database::InsertUser(const std::string &username, const std::string &normalized)
{
    try
    {
        std::unique_ptr<sql::PreparedStatement> stmt(
            con->prepareStatement(
                "INSERT INTO usernames (username,normalized)"
                "VALUES (?,?)"));

        stmt->setString(1, username);
        stmt->setString(2, normalized);

        stmt->execute();

        return true;
    }
    catch (sql::SQLException &e)
    {
        return false;
    }
}
