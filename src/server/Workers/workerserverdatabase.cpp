#include "Workers/workerserverdatabase.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariantList>
#include <QThread>

using namespace DataBaseWork;

QMutex WorkerServerDataBase::_data_base_mutex(QMutex::Recursive);

const char* WorkerServerDataBase::TYPE_DATA_BASE = "QSQLITE";
const char* WorkerServerDataBase::DEFAULT_NAME_DATA_BASE = "data.sqlite";
const char* WorkerServerDataBase::DEFAULT_NAME_CONNECTION_DATA_BASE = "db_connection_";

const char* WorkerServerDataBase::INSERT_USER_REQUEST = "INSERT INTO users (name) VALUES (:name);";
const char* WorkerServerDataBase::DELETE_USER_REQUEST = "DELETE FROM users WHERE name = :name;";
const char* WorkerServerDataBase::SELECT_USER_REQUEST = "SELECT name FROM users;";
const char* WorkerServerDataBase::SELECT_ANY_USER_REQUEST = "SELECT name FROM users WHERE name = :name;";

const char* WorkerServerDataBase::INSERT_DIR_REQUEST = "INSERT INTO dirs (name,path) VALUES (:name,:path);";
const char* WorkerServerDataBase::DELETE_DIR_REQUEST = "DELETE FROM dirs WHERE name = :name;";
const char* WorkerServerDataBase::SELECT_DIR_REQUEST = "SELECT path FROM dirs  WHERE name = :name;";

const char* WorkerServerDataBase::INSERT_FILES_REQUEST = "INSERT INTO files (name,path,created_data,last_modified_data,size) "
                                                         "VALUES (:name,:path,:created_data,:last_modified_data,:size);";
const char* WorkerServerDataBase::DELETE_FILES_REQUEST = "DELETE FROM files WHERE name = :name;";
const char* WorkerServerDataBase::SELECT_FILES_REQUEST = "SELECT path,created_data,last_modified_data,size FROM files WHERE name = :name;";

const char* WorkerServerDataBase::INSERT_ADDR_INFO_USER_REQUEST = "INSERT INTO user_addr_info (name,port,addr) VALUES (:name,:port,:addr);";
const char* WorkerServerDataBase::DELETE_ADDR_INFO_REQUEST = "DELETE FROM user_addr_info WHERE name = :name;";
const char* WorkerServerDataBase::UPDATE_ADDR_USER_REQUEST = "UPDATE user_addr_info SET addr = :addr WHERE name = :name;";
const char* WorkerServerDataBase::UPDATE_PORT_USER_REQUEST = "UPDATE user_addr_info SET port = :port WHERE name = :name;";
const char* WorkerServerDataBase::SELECT_ANY_ADDR_INFO_REQUEST = "SELECT addr, port FROM user_addr_info WHERE name = :name;";

QString WorkerServerDataBase::get_new_name_connection()
{
    int id = 0;
    auto temp = QThread::currentThreadId();
    memcpy(&id,&temp,sizeof (id));
    return DEFAULT_NAME_CONNECTION_DATA_BASE + QString::number(id);
}

void WorkerServerDataBase::remove_connection(const QString &name_connection)
{
    QSqlDatabase::removeDatabase(name_connection);
}

QStringList WorkerServerDataBase::get_all_user()const
{
    QMutexLocker lock(&_data_base_mutex);

    QStringList res;
    QSqlQuery query(_base);
    query.prepare(SELECT_USER_REQUEST);
    if(_base.isOpen() && query.exec())
    {
        while(query.next())
        {
            res += query.value("name").toString();
        }
    }
    return res;
}

bool WorkerServerDataBase::insert_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    QSqlQuery query(_base);
    query.prepare(INSERT_USER_REQUEST);
    query.bindValue(":name",user);
    if(!_base.isOpen() || !query.exec())
    {
        return false;
    }
    return true;
}

bool WorkerServerDataBase::delete_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    QSqlQuery query(_base);
    query.prepare(DELETE_USER_REQUEST);
    query.bindValue(":name",user);
    if(!_base.isOpen() || !is_user(user) || !query.exec())
    {
        return false;
    }
    return true;
}

bool WorkerServerDataBase::is_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    QSqlQuery query(_base);
    query.prepare(SELECT_ANY_USER_REQUEST);
    query.bindValue(":name",user);
    if(!_base.isOpen() || !query.exec())
    {
        return false;
    }
    else
    {
        if(!query.next())
        {
            return false;
        }
        return true;
    }
}

bool WorkerServerDataBase::insert_data_dir_user(const QString &user,const QStringList &dirs)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen() && is_user(user))
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(INSERT_DIR_REQUEST);
        for(auto &dir : dirs)
        {
            query.bindValue(":name",user);
            query.bindValue(":path",dir);
            if(!query.exec())
            {
                _base.rollback();
                return false;
            }
        }
        _base.commit();
        return true;
    }
    return false;
}

bool WorkerServerDataBase::delete_data_dir_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen())
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(DELETE_DIR_REQUEST);
        query.bindValue(":name",user);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

WorkerServerDataBase::DirsPath WorkerServerDataBase::get_data_dir_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    QStringList res;
    QSqlQuery query(_base);
    query.prepare(SELECT_DIR_REQUEST);
    query.bindValue(":name",user);
    if(_base.isOpen() && query.exec())
    {
        while(query.next())
        {
            res += query.value("path").toString();
        }
    }
    return res;
}

bool WorkerServerDataBase::insert_data_files_user(const QString &user,const WorkerServerDataBase::FileMetaData &data)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen() && is_user(user))
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(INSERT_FILES_REQUEST);
        for(auto it = data.begin(); it != data.end(); it++)
        {
            query.bindValue(":name",user);
            query.bindValue(":path",it.key());
            query.bindValue(":created_data",std::get<INDEX_CREATED>(it.value()));
            query.bindValue(":last_modified_data",std::get<INDEX_LAST_MODIFIED>(it.value()));
            query.bindValue(":size",QString::number(std::get<INDEX_SIZE>(it.value())));

            if(!query.exec())
            {
                _base.rollback();
                return false;
            }
        }
        _base.commit();
        return true;
    }
    return false;
}

bool WorkerServerDataBase::delete_data_files_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen())
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(DELETE_FILES_REQUEST);
        query.bindValue(":name",user);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

WorkerServerDataBase::FileMetaData WorkerServerDataBase::get_data_files_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    FileMetaData res;
    QSqlQuery query(_base);
    query.prepare(SELECT_FILES_REQUEST);
    query.bindValue(":name",user);
    if(_base.isOpen() && query.exec())
    {
        while(query.next())
        {
            auto path = query.value("path").toString();
            quint64 size = static_cast<quint64>(query.value("size").toString().toDouble());
            auto created_data = query.value("created_data").toString();
            auto last_modified_data = query.value("last_modified_data").toString();
            FileCharacteristics data = std::make_tuple(size,created_data,last_modified_data);
            res.insert(path,data);
        }
    }
    return res;
}

bool WorkerServerDataBase::is_user_info(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    return get_addr_info_user(user).first == "null" ? false : true;
}

bool WorkerServerDataBase::insert_addr_info_user(const QString &user,const QString &addr, quint16 port)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen() && is_user(user))
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(INSERT_ADDR_INFO_USER_REQUEST);
        query.bindValue(":name",user);
        query.bindValue(":port",port);
        query.bindValue(":addr",addr);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

bool WorkerServerDataBase::change_addr_user(const QString &user,const QString &addr)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen() && is_user(user))
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(UPDATE_ADDR_USER_REQUEST);
        query.bindValue(":name",user);
        query.bindValue(":addr",addr);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

bool WorkerServerDataBase::change_port_user(const QString &user,quint16 port)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen() && is_user(user))
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(UPDATE_PORT_USER_REQUEST);
        query.bindValue(":name",user);
        query.bindValue(":port",port);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

bool WorkerServerDataBase::delete_addr_info_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    if(_base.isOpen())
    {
        _base.transaction();
        QSqlQuery query(_base);
        query.prepare(DELETE_ADDR_INFO_REQUEST);
        query.bindValue(":name",user);
        if(!query.exec())
        {
            _base.rollback();
            return false;
        }
        _base.commit();
        return true;
    }
    return false;
}

QPair<QString,quint16> WorkerServerDataBase::get_addr_info_user(const QString &user)
{
    QMutexLocker lock(&_data_base_mutex);

    QPair<QString,quint16> res("null",0);
    QSqlQuery query(_base);
    query.prepare(SELECT_ANY_ADDR_INFO_REQUEST);
    query.bindValue(":name",user);
    if(_base.isOpen() && query.exec())
    {
        if(query.next())
        {
            res.first = query.value("addr").toString();
            res.second = static_cast<quint16>(query.value("port").toInt());
        }
    }
    return res;
}

WorkerServerDataBase::WorkerServerDataBase(const QString &connection_name)
{
    if(QSqlDatabase::contains(connection_name))
    {
        QSqlDatabase::database(connection_name);
    }
    else
    {
        _base = QSqlDatabase::addDatabase(TYPE_DATA_BASE,connection_name);
        _base.setDatabaseName(DEFAULT_NAME_DATA_BASE);
        _base.open();
    }
}

WorkerServerDataBase::~WorkerServerDataBase()
{
    _base.close();
}
