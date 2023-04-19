#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;
/*
为什么要创建连接池？
若系统需要频繁的访问数据库，则需要频繁创建和断开数据库连接，而创建数据库连接是一个很耗时的操作，也容易对数据库造成安全隐患
在程序初始化的时候，集中创建多个数据库连接，并进行集中管理，供程序使用，可以保证较快的数据库读写速度，更加安全可靠。
*/
class connection_pool
{
//单例模式：懒汉
public:
	MYSQL *GetConnection();				 //获取数据库连接
	bool ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取当前空闲连接数量
	void DestroyPool();					 //销毁所有连接

	//单例模式
	static connection_pool *GetInstance();

	void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log); 

private:
	connection_pool();
	~connection_pool();

	int m_MaxConn;  //最大连接数
	int m_CurConn;  //当前已使用的连接数
	int m_FreeConn; //当前空闲的连接数
	locker lock;
	list<MYSQL *> connList; //连接池
	sem reserve; //当前是否有空闲连接（信号量value初始化为m_FreeConn）

public:
	string m_url;			 //主机地址
	string m_Port;		 //数据库端口号
	string m_User;		 //登陆数据库用户名
	string m_PassWord;	 //登陆数据库密码
	string m_DatabaseName; //使用数据库名
	int m_close_log;	//日志开关
};

//将数据库连接的获取与释放通过RAII机制封装，避免手动释放。
class connectionRAII{

public:
	connectionRAII(MYSQL **con, connection_pool *connPool); //分配连接
	~connectionRAII(); //释放连接，将数据库连接放回队列
	
private:
	MYSQL *conRAII; //保存传入的数据库连接，便于后续释放（<==>重新放入数据库连接队列）
	connection_pool *poolRAII; //保存数据库连接池对象，同步连接池操作
};

#endif
