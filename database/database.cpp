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
        std::cerr << e.what() << '\n';
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
        std::cerr << "Connect failed: " << e.what() << "\nError code: " << e.getErrorCode() << '\n';

        return false;
    }
}

std::vector<user> Database::GetUsers()
{
    try
    {
        std::vector<user> users;

        std::unique_ptr<sql::Statement> stmt(
            con->createStatement());

        std::unique_ptr<sql::ResultSet> res(
            stmt->executeQuery(
                "SELECT id, username, normalized FROM usernames"));

        while (res->next())
        {
            user cur_user;

            cur_user.id = res->getInt64("id");
            cur_user.username = res->getString("username");
            cur_user.normalized = res->getString("normalized");

            users.push_back(cur_user);
        }

        return users;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "GetUsers failed: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << ", SQLState: " << e.getSQLState() << ")\n";

        return {};
    }
}
