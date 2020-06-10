#ifndef WORKERSERVERDATABASE_H
#define WORKERSERVERDATABASE_H

#include <QSqlDatabase>
#include <QStringList>
#include <QMap>
#include <QMutex>

namespace DataBaseWork
{
    class WorkerServerDataBase
    {
    public:

        using FileCharacteristics = std::tuple<quint64,QString,QString>;
        using FileMetaData = QMap<QString,FileCharacteristics>;
        using DirsPath = QStringList;

        WorkerServerDataBase(const QString &connection_name = DEFAULT_NAME_CONNECTION_DATA_BASE);
        virtual ~WorkerServerDataBase();

        bool insert_user(const QString &user);
        bool delete_user(const QString &user);
        bool is_user(const QString &user);
        QStringList get_all_user()const;

        bool insert_data_dir_user(const QString &user,const QStringList &dirs);
        bool delete_data_dir_user(const QString &user);
        DirsPath get_data_dir_user(const QString &user);

        bool insert_data_files_user(const QString &user, const FileMetaData &data);
        bool delete_data_files_user(const QString &user);
        FileMetaData get_data_files_user(const QString &user);

        bool insert_addr_info_user(const QString &user,const QString &addr, quint16 port);
        bool is_user_info(const QString &user);
        bool change_addr_user(const QString &user,const QString &addr);
        bool change_port_user(const QString &user,quint16 port);
        bool delete_addr_info_user(const QString &user);
        QPair<QString,quint16> get_addr_info_user(const QString &user);

        static QString get_new_name_connection();
        static void remove_connection(const QString &name_connection = DEFAULT_NAME_CONNECTION_DATA_BASE);

    private:
        QSqlDatabase _base;

        static QMutex _data_base_mutex;

        static const char* DEFAULT_NAME_DATA_BASE;
        static const char* TYPE_DATA_BASE;
        static const char* DEFAULT_NAME_CONNECTION_DATA_BASE;

        static const char* INSERT_USER_REQUEST;
        static const char* DELETE_USER_REQUEST;
        static const char* SELECT_USER_REQUEST;
        static const char* SELECT_ANY_USER_REQUEST;

        static const char* INSERT_DIR_REQUEST;
        static const char* DELETE_DIR_REQUEST;
        static const char* SELECT_DIR_REQUEST;

        static const char* INSERT_FILES_REQUEST;
        static const char* DELETE_FILES_REQUEST;
        static const char* SELECT_FILES_REQUEST;

        static const char* INSERT_ADDR_INFO_USER_REQUEST;
        static const char* UPDATE_ADDR_USER_REQUEST;
        static const char* UPDATE_PORT_USER_REQUEST;
        static const char* SELECT_ANY_ADDR_INFO_REQUEST;
        static const char* DELETE_ADDR_INFO_REQUEST;

        static constexpr int INDEX_SIZE = 0;
        static constexpr int INDEX_CREATED = 1;
        static constexpr int INDEX_LAST_MODIFIED = 2;
    };
}


#endif // WORKERSERVERDATABASE_H
