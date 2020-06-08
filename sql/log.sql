/*
 Navicat Premium Data Transfer

 Source Server         : lokal
 Source Server Type    : MySQL
 Source Server Version : 100136
 Source Host           : localhost:3306
 Source Schema         : log

 Target Server Type    : MySQL
 Target Server Version : 100136
 File Encoding         : 65001

 Date: 12/05/2019 17:11:53
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for bootlog
-- ----------------------------
DROP TABLE IF EXISTS `bootlog`;
CREATE TABLE `bootlog`  (
  `time` datetime(0) NOT NULL,
  `hostname` char(128) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT 'UNKNOWN',
  `channel` tinyint(1) NOT NULL DEFAULT 0
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for change_name
-- ----------------------------
DROP TABLE IF EXISTS `change_name`;
CREATE TABLE `change_name`  (
  `pid` int(11) NULL DEFAULT NULL,
  `old_name` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `new_name` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `time` datetime(0) NULL DEFAULT NULL,
  `ip` varchar(20) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for command_log
-- ----------------------------
DROP TABLE IF EXISTS `command_log`;
CREATE TABLE `command_log`  (
  `userid` int(11) NOT NULL DEFAULT 0,
  `server` int(11) NOT NULL DEFAULT 0,
  `ip` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `port` int(6) NOT NULL DEFAULT 0,
  `username` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `command` text CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL,
  `date` datetime(0) NOT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for cube
-- ----------------------------
DROP TABLE IF EXISTS `cube`;
CREATE TABLE `cube`  (
  `pid` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `time` datetime(0) NOT NULL,
  `x` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `y` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `item_vnum` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `item_uid` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `item_count` int(5) UNSIGNED NOT NULL DEFAULT 0,
  `success` tinyint(1) NOT NULL DEFAULT 0,
  INDEX `pid`(`pid`) USING BTREE,
  INDEX `item_vnum`(`item_vnum`) USING BTREE,
  INDEX `item_uid`(`item_uid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for dragon_slay_log
-- ----------------------------
DROP TABLE IF EXISTS `dragon_slay_log`;
CREATE TABLE `dragon_slay_log`  (
  `guild_id` int(11) UNSIGNED NOT NULL,
  `vnum` int(11) UNSIGNED NOT NULL,
  `start_time` timestamp(0) NOT NULL,
  `end_time` timestamp(0) NOT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for fish_log
-- ----------------------------
DROP TABLE IF EXISTS `fish_log`;
CREATE TABLE `fish_log`  (
  `time` datetime(0) NOT NULL,
  `player_id` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `map_index` tinyint(4) NOT NULL DEFAULT 0,
  `fish_id` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `fishing_level` int(11) NOT NULL DEFAULT 0,
  `waiting_time` int(11) NOT NULL DEFAULT 0,
  `success` tinyint(4) NOT NULL DEFAULT 0,
  `size` smallint(6) NOT NULL DEFAULT 0
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for goldlog
-- ----------------------------
DROP TABLE IF EXISTS `goldlog`;
CREATE TABLE `goldlog`  (
  `date` varchar(10) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '0000-00-00',
  `time` varchar(8) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '00:00:00',
  `pid` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `what` int(11) NOT NULL DEFAULT 0,
  `how` set('BUY','SELL','SHOP_SELL','SHOP_BUY','EXCHANGE_TAKE','EXCHANGE_GIVE','QUEST') CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL,
  `hint` varchar(50) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL,
  INDEX `date_idx`(`date`) USING BTREE,
  INDEX `pid_idx`(`pid`) USING BTREE,
  INDEX `what_idx`(`what`) USING BTREE,
  INDEX `how_idx`(`how`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for hack_crc_log
-- ----------------------------
DROP TABLE IF EXISTS `hack_crc_log`;
CREATE TABLE `hack_crc_log`  (
  `time` datetime(0) NOT NULL,
  `login` char(16) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `name` char(24) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `ip` char(15) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `server` char(100) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `why` char(255) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `crc` int(11) NOT NULL DEFAULT 0
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for hack_log
-- ----------------------------
DROP TABLE IF EXISTS `hack_log`;
CREATE TABLE `hack_log`  (
  `time` datetime(0) NOT NULL,
  `login` char(16) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `name` char(24) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `ip` char(15) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `server` char(100) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT '',
  `why` char(255) CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT ''
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for hackshield_log
-- ----------------------------
DROP TABLE IF EXISTS `hackshield_log`;
CREATE TABLE `hackshield_log`  (
  `pid` int(11) UNSIGNED NOT NULL DEFAULT 0,
  `login` varchar(32) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `account_id` int(11) UNSIGNED NOT NULL,
  `name` varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `time` datetime(0) NOT NULL,
  `reason` varchar(25) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for levellog
-- ----------------------------
DROP TABLE IF EXISTS `levellog`;
CREATE TABLE `levellog`  (
  `name` varchar(50) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `level` int(11) NULL DEFAULT NULL,
  `time` datetime(0) NULL DEFAULT NULL,
  `account_id` int(11) NOT NULL,
  `pid` int(11) NULL DEFAULT NULL,
  `playtime` int(11) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for log
-- ----------------------------
DROP TABLE IF EXISTS `log`;
CREATE TABLE `log`  (
  `type` enum('ITEM','CHARACTER') CHARACTER SET latin5 COLLATE latin5_turkish_ci NOT NULL DEFAULT 'ITEM',
  `time` datetime(0) NOT NULL,
  `who` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `x` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `y` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `what` int(11) NOT NULL DEFAULT 0,
  `how` varbinary(50) NOT NULL DEFAULT '',
  `hint` varbinary(70) NULL DEFAULT NULL,
  `ip` varbinary(20) NULL DEFAULT NULL,
  `vnum` int(11) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for loginlog
-- ----------------------------
DROP TABLE IF EXISTS `loginlog`;
CREATE TABLE `loginlog`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `is_gm` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `login_time` datetime(0) NULL DEFAULT NULL,
  `channel` int(11) NULL DEFAULT NULL,
  `account_id` int(11) NULL DEFAULT NULL,
  `pid` int(11) NULL DEFAULT NULL,
  `hwid` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `mac` char(18) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `cname` char(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `ip` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `logout_time` datetime(0) NULL DEFAULT NULL,
  `playtime` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1348683 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for loginlog2
-- ----------------------------
DROP TABLE IF EXISTS `loginlog2`;
CREATE TABLE `loginlog2`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `is_gm` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `login_time` datetime(0) NULL DEFAULT NULL,
  `channel` int(11) NULL DEFAULT NULL,
  `account_id` int(11) NULL DEFAULT NULL,
  `pid` int(11) NULL DEFAULT NULL,
  `client_version` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `ip` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  `logout_time` datetime(0) NULL DEFAULT NULL,
  `playtime` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 34641 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for money_log
-- ----------------------------
DROP TABLE IF EXISTS `money_log`;
CREATE TABLE `money_log`  (
  `time` datetime(0) NULL DEFAULT NULL,
  `DELAYED` int(11) NULL DEFAULT NULL,
  `vnum` int(11) NULL DEFAULT 0,
  `pid` int(11) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Fixed;

-- ----------------------------
-- Table structure for offlineshop_log
-- ----------------------------
DROP TABLE IF EXISTS `offlineshop_log`;
CREATE TABLE `offlineshop_log`  (
  `account_id` int(11) NOT NULL,
  `action` set('PUT','PUT_REMOTE','GIVE_BACK','FETCH','FETCH_MONEY','SELL') CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT '',
  `item` varchar(64) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL,
  `time` datetime(0) NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP(0)
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for pcbang_loginlog
-- ----------------------------
DROP TABLE IF EXISTS `pcbang_loginlog`;
CREATE TABLE `pcbang_loginlog`  (
  `time` datetime(0) NULL DEFAULT NULL,
  `pcbang_id` int(20) NULL DEFAULT NULL,
  `ip` varchar(30) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `pid` int(20) NULL DEFAULT NULL,
  `play_time` int(20) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for quest_reward_log
-- ----------------------------
DROP TABLE IF EXISTS `quest_reward_log`;
CREATE TABLE `quest_reward_log`  (
  `1` varchar(50) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL,
  `2` int(11) NULL DEFAULT NULL,
  `3` int(11) NULL DEFAULT NULL,
  `4` int(11) NULL DEFAULT NULL,
  `5` int(11) NULL DEFAULT NULL,
  `6` int(11) NULL DEFAULT NULL,
  `7` time(0) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for refinelog
-- ----------------------------
DROP TABLE IF EXISTS `refinelog`;
CREATE TABLE `refinelog`  (
  `Id` int(11) NULL DEFAULT NULL,
  `pid` int(11) NULL DEFAULT NULL,
  `item_name` varchar(50) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL,
  `item_id` int(11) NULL DEFAULT NULL,
  `step` int(11) NULL DEFAULT NULL,
  `time` time(0) NULL DEFAULT NULL,
  `is_success` int(11) NULL DEFAULT NULL,
  `setType` varchar(50) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for shout_log
-- ----------------------------
DROP TABLE IF EXISTS `shout_log`;
CREATE TABLE `shout_log`  (
  `time` datetime(0) NULL,
  `channel` tinyint(4) NULL DEFAULT NULL,
  `empire` tinyint(4) NULL DEFAULT NULL,
  `shout` text CHARACTER SET utf8 COLLATE utf8_general_ci NULL,
  INDEX `time_idx`(`time`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for speed_hack
-- ----------------------------
DROP TABLE IF EXISTS `speed_hack`;
CREATE TABLE `speed_hack`  (
  `pid` int(11) NULL DEFAULT NULL,
  `time` time(0) NULL DEFAULT NULL,
  `x` int(11) NULL DEFAULT NULL,
  `y` int(11) NULL DEFAULT NULL,
  `hack_count` varchar(20) CHARACTER SET latin5 COLLATE latin5_turkish_ci NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = latin5 COLLATE = latin5_turkish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for whisper_log
-- ----------------------------
DROP TABLE IF EXISTS `whisper_log`;
CREATE TABLE `whisper_log`  (
  `time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0),
  `from_msg` varchar(24) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT 'error',
  `to_msg` varchar(24) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT 'error',
  `msg_text` varchar(250) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT 'empty_msg_error'
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
