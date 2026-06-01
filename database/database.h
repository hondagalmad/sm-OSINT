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

