-- bluexg_v1 Database Create Script 202100803

CREATE DATABASE `bluexg_v1` /*!40100 DEFAULT CHARACTER SET utf8mb3 */;


-- bluexg_v1 User Add

CREATE USER 'bxrgadm'@'%' IDENTIFIED BY 'Whdms9500!'; 
CREATE USER 'bxrgweb'@'%' IDENTIFIED BY 'Whdms9500!'; 
CREATE USER 'bxrgsvr'@'%' IDENTIFIED BY 'Whdms9500!'; 


-- bluexg_v1 Policy Add

GRANT ALL PRIVILEGES ON bluexg_v1.* TO 'bxrgadm'@'%';
GRANT ALL PRIVILEGES ON bluexg_v1.* TO 'bxrgweb'@'%';
GRANT ALL PRIVILEGES ON bluexg_v1.* TO 'bxrgsvr'@'%'; 

FLUSH PRIVILEGES;

USE bluexg_v1;

-- bluexg_v1.tb_board_notice definition

CREATE TABLE `tb_board_notice` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Notice Index',
  `type` varchar(45) DEFAULT NULL COMMENT 'Notice Type',
  `code` varchar(45) DEFAULT NULL COMMENT 'Notice Code',
  `userId` varchar(45) NOT NULL COMMENT 'UserId',
  `title` varchar(45) NOT NULL COMMENT 'Notice Title',
  `content` mediumtext NOT NULL COMMENT 'Notice Content',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `hitCount` int(11) DEFAULT 0 COMMENT 'Notice HitCount',
  `status` int(11) NOT NULL COMMENT 'Notice Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_board_noticeFile definition

CREATE TABLE `tb_board_noticeFile` (
  `fileIdx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Notice File Index',
  `idx` int(11) DEFAULT NULL COMMENT 'Notice Index',
  `orgFileName` varchar(400) NOT NULL COMMENT 'File OriginalFileName',
  `saveFileName` varchar(400) NOT NULL COMMENT 'File SaveFileName',
  `fileSize` int(11) DEFAULT NULL COMMENT 'File Size',
  PRIMARY KEY (`fileIdx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_board_qna definition

CREATE TABLE `tb_board_qna` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Qna Index',
  `type` varchar(10) DEFAULT NULL COMMENT 'Notice Type',
  `code` varchar(45) DEFAULT NULL COMMENT 'Notice Code',
  `userId` varchar(45) NOT NULL COMMENT 'UserId',
  `title` varchar(45) NOT NULL COMMENT 'Title',
  `content` mediumtext NOT NULL COMMENT 'Content',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `private` int(11) DEFAULT NULL COMMENT 'Public/Private',
  `pIdx` int(11) DEFAULT NULL COMMENT 'pIdx',
  `status` int(11) NOT NULL COMMENT 'Status',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_info_code definition

CREATE TABLE `tb_info_code` (
  `pKey` varchar(4) NOT NULL COMMENT 'Parents Key',
  `cKey` varchar(16) NOT NULL COMMENT 'Child Key',
  `kValue` varchar(45) DEFAULT NULL COMMENT 'Korean Value',
  `eValue` varchar(45) DEFAULT NULL COMMENT 'English Value',
  `state` varchar(1) DEFAULT NULL COMMENT 'Code Status Flage ( A:allow, I:ignore )',
  `info` varchar(45) DEFAULT NULL COMMENT 'Information',
  PRIMARY KEY (`pKey`,`cKey`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- bluexg_v1.tb_info_user definition

CREATE TABLE `tb_info_user` (
  `userId` varchar(45) NOT NULL COMMENT 'User ID',
  `userPwd` varchar(64) DEFAULT NULL COMMENT 'User Password',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'User Uniq ID',
  `userName` varchar(45) DEFAULT NULL COMMENT 'User Name',
  `rankCode` varchar(32) DEFAULT NULL COMMENT 'Rank Code',
  `positionCode` varchar(32) DEFAULT NULL COMMENT 'Position Code',
  `deptCode` varchar(32) DEFAULT NULL COMMENT 'Dept Code',
  `officeTel` varchar(45) DEFAULT NULL COMMENT 'Office Tel',
  `officeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Office Address Code',
  `officeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Office Address Detail',
  `phone` varchar(45) DEFAULT NULL COMMENT 'Phone Number',
  `email` varchar(45) DEFAULT NULL COMMENT 'Email',
  `homeTel` varchar(45) DEFAULT NULL COMMENT 'Home Tel',
  `homeAddrCode` varchar(45) DEFAULT NULL COMMENT 'Home Address Code',
  `homeAddrDetail` varchar(45) DEFAULT NULL COMMENT 'Home Address Detail',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Data Time',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Update Data Time',
  `status` int(11) DEFAULT NULL COMMENT 'Status',
  PRIMARY KEY (`userId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_log_change definition

CREATE TABLE `tb_log_change` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Event Log Idx',
  `type` varchar(45) DEFAULT NULL COMMENT 'Connect Media Type',
  `plugInOut` datetime(3) NOT NULL COMMENT 'PlugInOut',
  `userId` varchar(45) NOT NULL COMMENT 'UserId',
  `uuid` varchar(64) DEFAULT NULL COMMENT 'UUID',
  `state` int(11) NOT NULL DEFAULT 0 COMMENT 'State',
  `browser` varchar(255) NOT NULL COMMENT 'Browser',
  `ipAddr` varchar(100) NOT NULL COMMENT 'IpAddr',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_log_event_detail definition

CREATE TABLE `tb_log_event_detail` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'EventLog index',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Date',
  `appFrom` varchar(256) DEFAULT NULL COMMENT 'From',
  `appTo` varchar(256) DEFAULT NULL COMMENT 'To',
  `trFunction` varchar(45) DEFAULT NULL COMMENT 'Function',
  `accessToken` varchar(1024) DEFAULT NULL COMMENT 'TokenKey',
  `clientId` varchar(64) DEFAULT NULL COMMENT 'ClientId',
  `logType` varchar(45) DEFAULT NULL COMMENT 'Log Type',
  `fileName` varchar(255) DEFAULT NULL COMMENT 'FileName',
  `fileSize` int(255) DEFAULT NULL COMMENT 'FileSize',
  `filePath` varchar(255) DEFAULT NULL COMMENT 'FilePath',
  `logWork` varchar(45) DEFAULT NULL COMMENT 'Work',
  `eCode` varchar(45) DEFAULT NULL COMMENT 'Error Code',
  `logStat` varchar(45) DEFAULT NULL COMMENT 'Stat',
  `pDate` datetime(3) DEFAULT NULL COMMENT 'Process Date',
  `lsfCode` int(11) DEFAULT NULL COMMENT 'lsf Code',
  `lsfMesg` varchar(45) DEFAULT NULL COMMENT 'lsf Message',
  `pIdx` int(11) DEFAULT NULL COMMENT 'Law Data Idx',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_log_event_view definition

CREATE TABLE `tb_log_event_view` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'EventLog index',
  `sWho` varchar(64) DEFAULT NULL COMMENT 'Client App Name',
  `sWhen` datetime(3) DEFAULT NULL COMMENT 'Create Date',
  `sWhere` varchar(256) DEFAULT NULL COMMENT 'Client ID',
  `sWhat` varchar(45) DEFAULT NULL COMMENT 'Function',
  `sHow` varchar(1024) DEFAULT NULL COMMENT 'Process Message',
  `sWhy` varchar(45) DEFAULT NULL COMMENT 'Event Message',
  `cnt` int(11) DEFAULT NULL COMMENT 'Send Count',
  `cDate` datetime(3) DEFAULT NULL COMMENT 'Create Date',
  `pDate` datetime(3) DEFAULT NULL COMMENT 'Process Date',
  `pIdx` varchar(45) DEFAULT NULL COMMENT 'Detail Data Idx',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;


-- bluexg_v1.tb_log_event_law definition

CREATE TABLE `tb_log_event_law` (
  `idx` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',
  `state` varchar(1) NOT NULL COMMENT 'Log State',
  `cDate` datetime(3) NOT NULL COMMENT 'Log Create DateTime',
  `uDate` datetime(3) DEFAULT NULL COMMENT 'Log Update DateTime',
  `iData` varchar(20480) NOT NULL COMMENT 'Log Law Data',
  `rCount` int(11) DEFAULT NULL COMMENT 'Parser Retry Count',
  PRIMARY KEY (`idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

