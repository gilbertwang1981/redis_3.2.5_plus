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

# the directory path of the plugin dynamic linked library,all the dynamic linked library will be put into this folder
# when the redis is started, these dynamic linked libraries will be loaded automatically.
export TS_PLUGIN_PATH=/root/ts_plugin

# the prototype of the plugin dynamic linked library as follows (ts_plgin.c)
# int ts_plugin_next(char * counter_name , int second ,
#        int counter_value , int eof , int bof) {
#        printf("%s %d %d %d %d\n" , counter_name , second , counter_value , eof , bof);
#        return 0;
# }

# use the following command to compile
# gcc -fPIC -shared ts_plugin.c -o libts_plugin.so