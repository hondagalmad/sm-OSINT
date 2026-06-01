#pragma once
#include <mysql_connection.h>
#include <mysql_driver.h>

// The main data of users
struct user
{
    int id;
    std::string username;
    std::string normalized;
};

class Database
{
private:
    // MySQL driver && connection
    sql::mysql::MySQL_Driver *driver;
    std::unique_ptr<sql::Connection> con;

public:
    Database();

    bool connect(const std::string &host, const std::string &user, const std::string &password, const std::string &database);

    void disconnect();

    bool InsertUser(const std::string &usernmae, const std::string &normalized);

    std::vector<user> GetUsers();
};
