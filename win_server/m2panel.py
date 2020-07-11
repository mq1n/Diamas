#-*- coding: utf-8 -*-
import sys, os, time

def sys_argc():
	return int(len(sys.argv)) - 1

def sys_argv():
	return sys.argv[1:]

class WinSFHelper:
	def __init__(self):
		self.work_type = 0
		self.build_type = "<undefined>"
		self.arch = "86" # Default: x86
		self.channel_count = 1
		self.root_path = os.getcwd()
		
		if os.path.isfile("{}\\syserr.txt".format(self.root_path)):
			os.remove("{}\\syserr.txt".format(self.root_path))

		if os.path.isfile("{}\\syslog.txt".format(self.root_path)):
			os.remove("{}\\syslog.txt".format(self.root_path))
		
		if sys_argc() < 2:
			self.show_usage()
			sys.exit(1)

		# Minimum arg count: 2
		# Usage: script.py <work_type> <core_build_type> [channel_count] [arch]

		args = sys_argv()
		self.sys_log("Args({}): {}".format(sys_argc(), args))
		
		self.set_work_type(args[0])
		self.set_build_type(args[1])
		if sys_argc() == 3:
			self.set_arch_type(args[2])
			self.channel_count = int(args[2])

		self.sys_log("Work type: {} Build type: {} Arch: {}".format(self.work_type, self.build_type, self.arch))

		if self.work_type < 1 or self.work_type > 4: 
			self.sys_err("Unknown work_type: {}".format(self.work_type))
			sys.exit(2)

		if self.build_type not in ["debug", "normal"]:
			self.sys_err("Unknown build_type: {}".format(self.build_type))
			sys.exit(3)

	def sys_log(self, data):
		with open("{}\\syslog.txt".format(self.root_path), "a") as f:
			f.write("SYS_LOG {}: {}\n".format(str(time.strftime("%H.%M.%S - %d.%m.%Y")), str(data)))

	def sys_err(self, data):
		print("SYS_ERR: {}".format(data))
		with open("{}\\syserr.txt".format(self.root_path), "a") as f:
			f.write("SYS_ERR {}: {}\n".format(str(time.strftime("%H.%M.%S - %d.%m.%Y")), str(data)))
	
	def set_work_type(self, type):
		if type in ["oyun", "oyunuac", "ac", "start", "game"]:
			self.work_type = 1
		elif type in ["temizle", "temiz", "log", "clear"]:
			self.work_type = 2
		elif type in ["kaldir", "remove", "sil"]:
			self.work_type = 3
		elif type in ["kur", "install", "yukle"]:
			self.work_type = 4
	
	def set_build_type(self, type):
		if type in ["normal", "n"]:
			self.build_type = "normal"
		elif type in ["debug", "d"]:
			self.build_type = "debug"

	def set_arch_type(self, type):
		if type in ["86", "x86"]:
			self.arch = "86"
		elif type in ["64", "x64", "x86_64"]:
			self.arch = "64"

	def start_worker(self):
		if self.work_type == 1:
			self.start_game()
		elif self.work_type == 2:
			self.clean_logs()
		elif self.work_type == 3:
			self.uninstall_game()
		elif self.work_type == 4:
			self.install_game()

	def start_game(self):
		self.sys_log("start_game triggered.")

		if self.channel_count < 1 or self.channel_count > 4:
			self.sys_err("Unknown channel_count: {}".format(self.channel_count))
			sys.exit(4)

		container_roots = ["\\auth\\", "\\db\\", "\\game99\\", "\\kanal\\"]
			
		if self.build_type == "normal":
			container_core = ["game.exe", "db.exe"]
		if self.build_type == "debug":
			container_core = ["game_d.exe", "db_d.exe"]
		
		for i in container_roots:
			target_path = "{}{}".format(self.root_path, i)
			os.chdir(target_path)

			coreFile = ""
			if str(i) == "\\db\\":
				coreFile = container_core[1]
			else:
				coreFile = container_core[0]

			target_file = "{}{}".format(target_path, coreFile)
			self.sys_log("'{}' starting...".format(target_file))

			try:
				os.startfile(target_file)
			except OSError as e:
				self.sys_err("'{}' could not started. Error: {}".format(target_file, e))
				pass

		self.sys_log("start_game completed.")
		
	def clean_logs(self):
		self.sys_log("clean_logs triggered.")

		garbage_container_roots = ["\\auth\\", "\\db\\", "\\game99\\", "\\kanal\\"]
		garbage_container_log = [
			"syserr.txt", "syslog.txt", "neterr.txt", "PTS.txt", "usage.txt",
			"packet_info.txt", "VERSION.txt", "mob_count", "p2p_packet_info.txt",
			"DEV_LOG.log"
		]
		
		for i in garbage_container_roots:
			file_list = os.listdir("{}{}".format(self.root_path, i))
			for file in file_list:
				file_name, ext = os.path.splitext(file)
				if ext == ".dmp" or file in garbage_container_log:
					target_file = "{}{}{}".format(self.root_path, i, file)
					self.sys_log("'{}' removing...".format(target_file))

					try:
						os.remove(target_file)
						self.sys_log("'{}' succesfully removed".format(target_file))
					except OSError as e:
						self.sys_err("'{}' could not removed. Error: {}".format(target_file, e))
						pass					
					
		self.sys_log("clean_logs completed.")
	
	def uninstall_game(self):
		self.sys_log("uninstall_game triggered.")

		garbage_container_roots =     ["\\auth\\", "\\db\\", "\\game99\\", "\\kanal\\"]
		garbage_container_dirs =      ["data", "locale", "log", "package", "mark"]
		if self.build_type == "debug":
			garbage_container_files = ["db_d.exe", "game_d.exe", "DevIL.dll"]
		elif self.build_type == "normal":
			garbage_container_files = ["db.exe", "game.exe", "DevIL.dll"]

		garbage_container_configs = [
			"\\ayar\\proto\\mob_proto.txt","\\ayar\\proto\\mob_names.txt", "\\ayar\\proto\\item_names.txt", "\\ayar\\proto\\item_proto.txt",
			"\\db\\mob_proto.txt","\\db\\mob_names.txt", "\\db\\item_names.txt", "\\db\\item_proto.txt", "\\db\\object_proto.txt",
			"\\ayar\\kanal_config.txt", "\\ayar\\game99_config.txt", "\\ayar\\auth_config.txt", "\\ayar\\db_conf.json"
		]

		for i in garbage_container_roots:
			for j in garbage_container_dirs:
				target_dir = "{}{}{}".format(self.root_path, i, j)

				try:
					os.rmdir(target_dir)
					self.sys_log("'{}' succesfully removed".format(target_dir))
				except OSError as e:
					self.sys_err("'{}' could not removed. Error: {}".format(target_dir, e))
					pass					
				
			for j in garbage_container_files:
				target_file = "{}{}{}".format(self.root_path, i, j)

				try:
					os.remove(target_file)
				except OSError as e:
					self.sys_err("'{}' could not removed. Error: {}".format(target_file, e))
					pass		
		
		for i in garbage_container_configs:
			target_file = "{}{}".format(self.root_path, i)

			try:
				os.remove(target_file)
			except OSError as e:
				self.sys_err("'{}' could not removed. Error: {}".format(target_file, e))
				pass		
		
		self.sys_log("uninstall_game completed.")
	
	def install_game(self):
		self.sys_log("install_game triggered.")

		if self.arch not in ["86", "64"]:
			self.sys_err("Unknown arch: {}".format(self.arch))
			sys.exit(5)

		if self.build_type == "normal":
			container_core = ["game.exe", "db.exe"]
		if self.build_type == "debug":
			container_core = ["game_d.exe", "db_d.exe"]

		symlink_targets_dir = [
			["kanal\\package",  "..\\share\\package"], ["kanal\\locale",  "..\\share\\locale"], ["kanal\\data",  "..\\share\\data"], ["kanal\\log",  "..\\share\\log"],       ["kanal\\mark", "..\\share\\mark"],
			["auth\\package",   "..\\share\\package"], ["auth\\locale",   "..\\share\\locale"], ["auth\\data",   "..\\share\\data"], ["auth\\log",   "..\\share\\auth\\log"], ["auth\\mark",  "..\\share\\mark"],
			["db\\package",     "..\\share\\package"], ["db\\locale",     "..\\share\\locale"], ["db\\data",     "..\\share\\data"], ["db\\log",     "..\\share\\db\\log"],
			["game99\\package", "..\\share\\package"], ["game99\\locale", "..\\share\\locale"], ["game99\\data", "..\\share\\data"], ["game99\\log", "..\\share\\game99\\log"]
		]
		
		symlink_targets_file = [
			["db\\{}".format(container_core[1]),     "..\\Bin\\db_cache.exe"],
			["auth\\{}".format(container_core[0]),   "..\\Bin\\game_server.exe"],
			["game99\\{}".format(container_core[0]), "..\\Bin\\game_server.exe"],
			["kanal\\{}".format(container_core[0]),  "..\\Bin\\game_server.exe"],

			["db\\item_names.txt",   "..\\share\\conf\\item_names.txt"], ["db\\mob_names.txt", "..\\share\\conf\\mob_names.txt"],
			["db\\item_proto.txt",   "..\\share\\conf\\item_proto.txt"], ["db\\mob_proto.txt", "..\\share\\conf\\mob_proto.txt"],
			["db\\object_proto.txt", "..\\share\\conf\\object_proto.txt"],

			["ayar\\auth_config.txt",  "auth\\CONFIG"],  ["ayar\\game99_config.txt", "game99\\CONFIG"],
			["ayar\\kanal_config.txt", "kanal\\CONFIG"], ["ayar\\db_conf.json",      "db\\CONFIG.json"],
		]			

		if self.arch == "64":
			symlink_targets_file.append(["auth\\DevIL.dll",   "..\\extern\\bin\\DevIL_x64.dll"])
			symlink_targets_file.append(["game99\\DevIL.dll", "..\\extern\\bin\\DevIL_x64.dll"])
			symlink_targets_file.append(["kanal\\DevIL.dll",  "..\\extern\\bin\\DevIL_x64.dll"])
		elif self.arch == "86":
			symlink_targets_file.append(["auth\\DevIL.dll",   "..\\extern\\bin\\DevIL.dll"])
			symlink_targets_file.append(["game99\\DevIL.dll", "..\\extern\\bin\\DevIL.dll"])
			symlink_targets_file.append(["kanal\\DevIL.dll",  "..\\extern\\bin\\DevIL.dll"])
			
		for i in symlink_targets_dir:
			target_file = "{} {}".format(i[0], i[1])

			try:
				os.system("mklink /D/J {}".format(target_file))
			except OSError as e:
				self.sys_err("'{}' could not linked. Error: {}".format(target_file, e))
				pass	

			self.sys_log("{} -> {} linked.".format(i[1], i[0]))

		for i in symlink_targets_file:
			target_file = "{} {}".format(i[0], i[1])

			try:
				os.system("mklink /H {}".format(target_file))
			except OSError as e:
				self.sys_err("'{}' could not linked. Error: {}".format(target_file, e))
				pass	

			self.sys_log("{} -> {} linked.".format(i[1], i[0]))		

		self.sys_log("install_game completed.")
	
	def show_usage(self):
		print("\nKullanim:\n\t"
			  "Oyunu acmak icin: oyun\n\t"
			  "Loglari Temizlemek icin: temizle\n\t"
			  "Kurulumu kaldirmak icin: kaldir\n\t"
			  "Kurulum yapmak icin: kur\n\t")

if __name__ == "__main__":
	helper = WinSFHelper()
	helper.start_worker()
