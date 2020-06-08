/*
 Navicat Premium Data Transfer

 Source Server         : local
 Source Server Type    : MySQL
 Source Server Version : 100411
 Source Host           : localhost:3306
 Source Schema         : account

 Target Server Type    : MySQL
 Target Server Version : 100411
 File Encoding         : 65001

 Date: 08/06/2020 23:49:57
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account`  (
  `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `login` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' COMMENT 'LOGIN_MAX_LEN=30',
  `password` varchar(42) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' COMMENT 'PASSWD_MAX_LEN=16; default 45 size',
  `social_id` varchar(7) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `email` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `securitycode` varchar(192) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `status` varchar(8) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'OK',
  `availDt` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `create_time` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `last_play` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `gold_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `silver_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `safebox_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `autoloot_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `fish_mind_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `marriage_fast_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `money_drop_rate_expire` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `real_name` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT '',
  `question1` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `answer1` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `question2` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `answer2` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `cash` int(11) NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `login`(`login`) USING BTREE,
  INDEX `social_id`(`social_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for block_exception
-- ----------------------------
DROP TABLE IF EXISTS `block_exception`;
CREATE TABLE `block_exception`  (
  `login` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT ''
) ENGINE = MyISAM CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of block_exception
-- ----------------------------
INSERT INTO `block_exception` VALUES ('NONE');

-- ----------------------------
-- Table structure for gametime
-- ----------------------------
DROP TABLE IF EXISTS `gametime`;
CREATE TABLE `gametime`  (
  `UserID` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `paymenttype` tinyint(2) NOT NULL DEFAULT 1,
  `LimitTime` int(11) UNSIGNED NULL DEFAULT 0,
  `LimitDt` datetime(0) NULL DEFAULT current_timestamp(),
  `Scores` int(11) NULL DEFAULT 0,
  PRIMARY KEY (`UserID`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for gametimeip
-- ----------------------------
DROP TABLE IF EXISTS `gametimeip`;
CREATE TABLE `gametimeip`  (
  `ipid` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(128) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `ip` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '000.000.000.000',
  `startIP` int(11) NOT NULL DEFAULT 0,
  `endIP` int(11) NOT NULL DEFAULT 255,
  `paymenttype` tinyint(2) NOT NULL DEFAULT 1,
  `LimitTime` int(11) NOT NULL DEFAULT 0,
  `LimitDt` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `readme` varchar(128) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  PRIMARY KEY (`ipid`) USING BTREE,
  UNIQUE INDEX `ip_uniq`(`ip`, `startIP`, `endIP`) USING BTREE,
  INDEX `ip_idx`(`ip`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 1 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for gametimelog
-- ----------------------------
DROP TABLE IF EXISTS `gametimelog`;
CREATE TABLE `gametimelog`  (
  `login` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `type` enum('IP_FREE','FREE','IP_TIME','IP_DAY','TIME','DAY') CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `logon_time` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `logout_time` datetime(0) NOT NULL DEFAULT current_timestamp(),
  `use_time` int(11) UNSIGNED NULL DEFAULT NULL,
  `ip` varchar(15) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '000.000.000.000',
  `server` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  INDEX `login_key`(`login`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for iptocountry
-- ----------------------------
DROP TABLE IF EXISTS `iptocountry`;
CREATE TABLE `iptocountry`  (
  `IP_FROM` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `IP_TO` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `COUNTRY_NAME` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL
) ENGINE = MyISAM CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of iptocountry
-- ----------------------------
INSERT INTO `iptocountry` VALUES ('0.0.0.0', '0.0.0.0', 'NONE');

-- ----------------------------
-- Table structure for string
-- ----------------------------
DROP TABLE IF EXISTS `string`;
CREATE TABLE `string`  (
  `name` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `text` text CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  PRIMARY KEY (`name`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
