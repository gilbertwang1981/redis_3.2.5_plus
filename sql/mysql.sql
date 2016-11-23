DROP TABLE redis_log;
DROP TABLE whitelist;

CREATE TABLE `redis_log` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `dkey` varchar(512) NOT NULL,
  `dvalue` varchar(8192) NOT NULL,
  `dexpire` int(11) NOT NULL,
  `dtimeunit` int(11) NOT NULL,
  `ip` varchar(64) NOT NULL,
  `cmd` int(8) NOT NULL,
  `create_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8

CREATE TABLE `whitelist` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `dkey` varchar(512) NOT NULL,
  `last_update_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8