# need to persist to mysql db,1:need or 0: not need
export REDIS_PERSISTENCE_FLAG=1
# persistence db host
export VIP_DB_HOST=10.100.70.40
# persistence db user name
export VIP_DB_USER=root
# persistence db password
export VIP_DB_PASS=root
# persistence db name
export VIP_DB_NAME=test

# the path of the dynamic linked lib, libmysqlclient.so.XXX
export LD_LIBRARY_PATH=/root/commonlib4c/thirdparty/mysql/lib

# the statistic command, if set 1, use incr system command, else use customized command PEG.
export REDIS_SWITCH_STAT_FLAG=1