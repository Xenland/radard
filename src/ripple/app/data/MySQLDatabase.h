#ifndef RIPPLE_MYSQLDATABASE_H_INCLUDED
#define RIPPLE_MYSQLDATABASE_H_INCLUDED

#include <mysql.h>
#include <boost/thread/tss.hpp>
#include <beast/module/core/text/StringPairArray.h>
#include <beast/utility/LeakChecked.h>
#include <ripple/app/data/DatabaseCon.h>

namespace ripple {

class MySQLStatement;
class MySQLThreadSpecificData;
class MySQLDatabase
    : public Database
    , private beast::LeakChecked <MySQLDatabase>
{
public:
    explicit MySQLDatabase (char const* host, std::uint32_t port, char const* username, char const* password, char const* database, bool asyncBatch);
    ~MySQLDatabase ();

    void connect (){};
    void disconnect (){};

    // returns true if the query went ok
    bool executeSQL (const char* sql, bool fail_okay);
    bool executeSQLBatch();
    
    bool batchStart() override;
    bool batchCommit() override;

    // tells you how many rows were changed by an update or insert
    std::uint64_t getNumRowsAffected ();

    // returns false if there are no results
    bool startIterRows (bool finalize);
    void endIterRows ();

    // call this after you executeSQL
    // will return false if there are no more rows
    bool getNextRow (bool finalize);
    
    bool beginTransaction() override;
    bool endTransaction() override;

    bool hasField(const std::string &table, const std::string &field) override;
    bool getNull (int colIndex);
    char* getStr (int colIndex, std::string& retStr);
    std::int32_t getInt (int colIndex);
    float getFloat (int colIndex);
    bool getBool (int colIndex);
    // returns amount stored in buf
    int getBinary (int colIndex, unsigned char* buf, int maxSize);
    Blob getBinary (int colIndex);
    std::uint64_t getBigInt (int colIndex);
private:
    bool getColNumber (const char* colName, int* retIndex);
    MySQLStatement *getStatement();
    
    std::uint32_t mPort;
    std::string mUsername;
    std::string mPassword;
    std::string mDatabase;
    bool mAsyncBatch;

    boost::thread_specific_ptr<MySQLStatement> mStmt;
    
    std::list<std::string> mSqlQueue;
    bool mThreadBatch = false;
    std::mutex mThreadBatchLock;
};
    
class MySQLStatement
{
public:
    MySQLStatement(char const* host, std::uint32_t port, char const* username, char const* password, char const* database);
    ~MySQLStatement();

    MYSQL *mConnection;
    std::list<std::string> mSqlQueue;
    bool mInBatch;
    bool mMoreRows;
    std::vector <std::string> mColNameTable;
    MYSQL_RES *mResult;
    MYSQL_ROW mCurRow;
};
    
class MySQLDatabaseCon
    : public DatabaseCon
{
public:
    MySQLDatabaseCon (beast::StringPairArray& mysqlParams, const char* initString[], int countInit);
};

} // ripple

#endif
