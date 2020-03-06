import subprocess
import os
import sys
import thread
import time
import traceback
import os, msvcrt

#msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
#msvcrt.setmode(sys.stderr.fileno(), os.O_BINARY)
#msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)

#p = os.popen(r"c:\cygwin64\bin\gdb.exe", "rw")
#p = subprocess.Popen(r"c:\cygwin64\bin\gdb.exe", stdin = subprocess.PIPE, stdout = subprocess.PIPE)
#os.system("gdb --interpreter=mi")
p = subprocess.Popen(["gdb "] + sys.argv[1:], stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
pp = open("prelog.txt","wb")
pp.write(repr(sys.argv[1:]))
pp.write("\n")
pp.write("1")
import code

class Log(object):
	def __init__(self, file_name):
		self.logfile = open(file_name,"wb")
		self.side_out = ""
		self.side_in = ""
		self.side_err = ""
	def write_side_in(self, data):
		assert len(data) == 1
		self.side_in += data
		if data == "\n":
			self.logfile.write("--> " + self.side_in)
			self.side_in = ""
		return
	def write_side_out(self, data):
		assert len(data) == 1
		self.side_out += data
		if data == "\n":
			if "script={" in self.side_out:
				self.logfile.write("<S- " + self.side_out)
			else:
				self.logfile.write("<-- " + self.side_out)
				sys.stdout.write(self.side_out)
				sys.stdout.flush()
			self.side_out = ""
		return
	def write_side_err(self, data):
		assert len(data) == 1
		self.side_err += data
		if data == "\n":
			self.logfile.write("<e- " + self.side_err)
			self.side_err = ""
		return

log = Log("log.txt")
def write_log(data, side):
	global log
	global pp
	if side == "err":
		log.write_side_err(data)
	if side == "out":
		log.write_side_out(data)
	else:
		log.write_side_in(data)
	log.logfile.flush()
	pp.write(data)
	pp.flush()

def input():
	global p
	global stop
	pp.write("3")
	while not stop:
		try:
			pp.write("5")
			d = sys.stdin.read(1)
			write_log(d, "in")
			pp.write("5.5")
			p.stdin.write(d)
			p.stdin.flush()
			pp.write("6")
		except:
			pp.write(" err1 ")
			pp.write(traceback.format_exec())
			stop = True
			raise
			return
def err():
	global stop
	global p
	while not stop:
		try:
			r = p.stderr.read(1)
			write_log(r, "err")
			pp.write(r)
			sys.stderr.write(r)
			sys.stderr.flush()
			raise
		except:
			pp.write(" err2 ")
			raise
			stop = True
			break

stop = False		
thread.start_new(input,())
thread.start_new(err,())
pp.write("2")
while not stop:
	try:
		r = p.stdout.read(1)
		write_log(r, "out")
		pp.write(r)
		#sys.stdout.write(r)
		#sys.stdout.flush()
	except:
		pp.write(" err3 ")
		raise
		stop = True
		break

pp.write("10")





