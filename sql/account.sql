/*
 Navicat Premium Data Transfer

 Source Server         : lokal
 Source Server Type    : MySQL
 Source Server Version : 100136
 Source Host           : localhost:3306
 Source Schema         : account

 Target Server Type    : MySQL
 Target Server Version : 100136
 File Encoding         : 65001

 Date: 12/05/2019 17:10:50
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `login` varchar(30) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `password` varchar(45) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `real_name` varchar(16) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `social_id` varchar(13) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `email` varchar(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `phone1` varchar(16) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `phone2` varchar(16) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `address` varchar(128) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `zipcode` varchar(7) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `ip` varchar(16) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `create_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0),
  `question1` varchar(48) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `answer1` varchar(48) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `question2` varchar(48) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `answer2` varchar(48) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT NULL,
  `is_testor` tinyint(1) NOT NULL DEFAULT 0,
  `status` varchar(8) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'OK',
  `securitycode` varchar(192) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT '',
  `newsletter` tinyint(1) NULL DEFAULT 0,
  `empire` tinyint(4) NOT NULL DEFAULT 0,
  `name_checked` tinyint(1) NOT NULL DEFAULT 0,
  `availDt` datetime(0) NOT NULL,
  `mileage` int(11) NOT NULL DEFAULT 0,
  `cash` int(11) NOT NULL DEFAULT 0,
  `gold_expire` datetime(0) NOT NULL,
  `silver_expire` datetime(0) NOT NULL,
  `safebox_expire` datetime(0) NOT NULL DEFAULT '2017-01-01 00:00:00',
  `autoloot_expire` datetime(0) NOT NULL DEFAULT '2017-01-01 00:00:00',
  `fish_mind_expire` datetime(0) NOT NULL DEFAULT '2017-01-01 00:00:00',
  `marriage_fast_expire` datetime(0) NOT NULL DEFAULT '2017-01-01 00:00:00',
  `money_drop_rate_expire` datetime(0) NOT NULL DEFAULT '2017-01-01 00:00:00',
  `ttl_cash` int(11) NOT NULL DEFAULT 0,
  `ttl_mileage` int(11) NOT NULL DEFAULT 0,
  `channel_company` varchar(30) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  `last_play` datetime(0) NULL DEFAULT NULL,
  `last_failed_attempt` datetime(0) NULL DEFAULT NULL,
  `failures` tinyint(2) NOT NULL DEFAULT 0,
  `web_ip` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `login`(`login`) USING BTREE,
  INDEX `social_id`(`social_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 2 CHARACTER SET = ascii COLLATE = ascii_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of account
-- ----------------------------
INSERT INTO `account` VALUES (1, 'koray', '*00A51F3F48415C7D4E8908980D443C29C69B60C9', NULL, '', '', NULL, NULL, NULL, NULL, NULL, '2019-05-11 15:59:01', NULL, NULL, NULL, NULL, 0, 'OK', '', 0, 0, 0, '0000-00-00 00:00:00', 0, 0, '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2017-01-01 00:00:00', '2017-01-01 00:00:00', '2017-01-01 00:00:00', '2017-01-01 00:00:00', '2017-01-01 00:00:00', 0, 0, '', '2019-05-11 17:30:10', NULL, 0, '');

SET FOREIGN_KEY_CHECKS = 1;
