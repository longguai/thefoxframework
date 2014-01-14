﻿/*
* @filename MySqlConnection.h
* @brief libmysql连接管理C++封装类，支持Windows和Linux
* @author macwe@qq.com
*/

#ifndef _THEFOX_MYSQL_CONNECTION_H_
#define _THEFOX_MYSQL_CONNECTION_H_

#include <mysql.h>
#include <base/Types.h>
#include <base/MutexLock.h>
#include <log/logger.h>
#include "MySqlResultSet.h"

namespace thefox
{

namespace mysql
{

/// @beirf mysql数据库连接类
class MySqlConnection
{
public:
	MySqlConnection(const String &host = "",
					const String &user = "",
					const String &passwd = "",
					const int port = 3306,
					const String &dbName = "")
		: _connPtr(NULL)
		, _host(host)
		, _user(user)
		, _passwd(passwd)
		, _port(port)
		, _dbName(dbName)
    {}
    
	~MySqlConnection()
    {
        close();
    }
    
	/// @brief 设置数据库连接参数
	/// @param[in] host 主机名或ip地址
	/// @param[in] user 用户名
	/// @param[in] passwd 密码
	/// @param[in] port 主机端口号
	/// @param[in] dbName 数据库名
	void setParam(const String &host, const String &user, const String &passwd, const int port, const String &dbName)
	{
		_host = host;
		_user = user;
		_passwd = passwd;
		_port = port;
		_dbName = dbName;
	}
	
	/// @brief 连接数据库
	/// @return 连接成功返回true，连接失败返回false
	bool open()
    { 
		if (isConnected())
			close();
			
        if (NULL == (_connPtr = mysql_init(&_conn))) 
		{
			LOG_ERROR << "mysql_init error, not enough memory!";
			return false;
		}

        if (NULL == mysql_real_connect(_connPtr, 
									_host.c_str(), 
									_user.c_str(), 
									_passwd.c_str(), 
									_dbName.c_str(), 
									_port, 
									NULL, 
									CLIENT_MULTI_STATEMENTS)) 
		{
			LOG_ERROR << "Failed to connect to database, Error: " << mysql_error(_connPtr);
			const char *ss = mysql_error(_connPtr);
			return false;
        }

        return true;
    }
	
	/// @beirf 关闭数据库连接
	void close()
    {
        if (isConnected()) 
		{
            mysql_close(_connPtr);
            _connPtr = NULL;
        }
    }
	
	/// @brief 返回数据库是否已经连接
	/// @return 已经连接返回true，未连接返回false
	bool isConnected() const 
	{ return NULL != _connPtr; }
    
	/// @brief 选择数据库
	/// @return 设置成功返回true，否则返回false
	bool selectDb(const String &dbName) 
	{ 
		_dbName = dbName; 
		if (0 != mysql_select_db(_connPtr, _dbName.c_str())) 
		{
			LOG_ERROR << "select database failed, Error: " << mysql_error(_connPtr);
			return false;
		}
		return true;
	}
	
	/// @brief 执行无返回结果集的查询语句
	/// @param[in] sql 待执行的sql语句
	/// @param[in|out] insertId 输出操作记录的ID
	///    (注：对于INSERT和UPDATE语句且设置AUTO_INCREMENT才有效)
	/// @return 执行成功返回true, 否则返回false
    bool exec(const String &sql, uint32_t *insertId = NULL)
    {
		if (!isConnected()) 
		{
			LOG_ERROR << "mysql query failed, database not connected! sql=" << sql;
			return false;
		}
		
		MutexLockGuard lock(_lock);
        if (0 != mysql_real_query(_connPtr, sql.c_str(), (unsigned long)sql.length()))
		{
			LOG_ERROR << "mysql query failed, sql=" << sql <<", Error: " << mysql_error(_connPtr);
            return false;
        }
        if (insertId)
            *insertId = (uint32_t)mysql_insert_id(_connPtr);
			
        return true;
    }
	
	/// @brief 执行SELECT语句
	/// @param[in] sql 待执行的sql语句
	/// @param[in|out] resultSet 返回的结果集
	/// @return 执行成功返回true, 否则返回false
	/// @sa resultSet
    bool query(const String &sql, MySqlResultSet &resultSet)
	{
		if (!isConnected())
		{
			LOG_ERROR << "mysql query failed, database not connected! sql=" << sql;
			return false;
		}
		
		MutexLockGuard lock(_lock);
        if (0 != mysql_real_query(_connPtr, sql.c_str(), sql.length()))
		{
			LOG_ERROR << "mysql query failed, sql=" << sql <<", Error: " << mysql_error(_connPtr);
            return false;
        }
		
		if (resultSet)
			mysql_free_result(resultSet);
		if (resultSet = mysql_store_result(_connPtr)) 
		{
			LOG_ERROR << "mysql query failed, sql=" << sql <<", Error: " << mysql_error(_connPtr);
		}
		return true;
	}
		
private:
	MYSQL _conn;
    MYSQL *_connPtr;
	MutexLock _lock;
	
	String _host;
	String _user;
	String _passwd;
	String _dbName;
	int _port;
};

} // namespace mysql

} // namesapce thefox

#endif // _THEFOX_MYSQL_CONNECTION_H_
