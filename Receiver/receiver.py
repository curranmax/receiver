
# Allows for computers without GMs, to run code without installing GM library
try:
	from gm import GM

	gm_supported = True
except ImportError:
	gm_supported = False

from socket import *
import time
import subprocess
import signal
import argparse

# In cases of no GM library, this won't be used
try:
	class WaitGM(GM):
		def __init__(self,wait_time):
			super(WaitGM,self).__init__()
			self.wait_time = wait_time

		def connectDevice(self):
			time.sleep(self.wait_time)
			super(WaitGM,self).connectDevice()
			time.sleep(self.wait_time)

		def disconnectDevice(self):
			time.sleep(self.wait_time)
			super(WaitGM,self).disconnectDevice()
			time.sleep(self.wait_time)

		def setValue(self,v):
			time.sleep(self.wait_time)
			rv = super(WaitGM,self).setValue(v)
			time.sleep(self.wait_time)
			return rv

		def getValue(self):
			time.sleep(self.wait_time)
			rv = super(WaitGM,self).getValue()
			time.sleep(self.wait_time)
			return rv
except NameError,e:
	if gm_supported:
		raise e
	else:
		pass

class SimpleFSO:
	def __init__(self,config_file,wait_time):
		self.val1 = None
		self.val2 = None
		self.is_val1 = True
		self.readConfig(config_file)
		self.gm = WaitGM(wait_time)
		self.gm.connectDevice()
		self.gm.setValue(self.val1)

	def readConfig(self,config_file):
		gm_fso_id = None
		fso_1_id = None
		fso_2_id = None
		vals = dict()
		f = open(config_file)
		for line in f:
			vs = line.split()
			if vs[0] == 'FSO':
				if len(vs) == 3 and vs[2] == 'GM':
					if gm_fso_id == None:
						gm_fso_id = vs[1]
					else:
						raise Exception('Incorrect network configuration')
				else:
					if fso_1_id == None:
						fso_1_id = vs[1]
					elif fso_2_id == None:
						fso_2_id = vs[1]
					else:
						raise Exception('Incorrect network configuration')
			if vs[0] == 'Link':
				vals[vs[1]] = int(vs[2])
		if None in [gm_fso_id,fso_1_id,fso_2_id]:
			raise Exception('Incorrect network configuration')
		self.val1 = vals[fso_1_id]
		self.val2 = vals[fso_2_id]

	def setWithOffset(self,offset):
		if self.is_val1:
			self.gm.setValue(self.val1 + offset)
		else:
			self.gm.setValue(self.val2 + offset)

	def reset(self):
		self.gm.setValue(self.val1)
		self.is_val1 = True

	def switchLink(self):
		if self.is_val1:
			self.gm.setValue(self.val2)
		else:
			self.gm.setValue(self.val1)
		self.is_val1 = not self.is_val1

class Sender:
	def __init__(self,fso,sock,foreign_port,test_port,verbose):
		self.fso = fso
		self.sock = sock
		self.foreign_port = foreign_port
		self.test_port = test_port
		self.v = verbose
		# send init message to get both addresses
		self.addr1 = None
		self.addr2 = None
		self.addr1,self.addr2 = self.sendBoth('init_message ' + str(test_port))

	def sendBoth(self,msg,msg2 = None):
		if self.useFSO():
			fso.reset()
		v1 = self.sendMsg(msg,self.addr1)
		if self.useFSO():
			fso.switchLink()
			if msg2 == None:
				v2 = self.sendMsg(msg,self.addr2)
			else:
				v2 = self.sendMsg(msg2,self.addr2)
			fso.switchLink()
		else:
			v2 = None
		return v1,v2

	def sendMsg(self,msg,to_addr):
		if to_addr == None:
			to_addr = ('<broadcast>',self.foreign_port)
		n_timeout = 0
		while True:
			if n_timeout >= 2:
				# self.tryWalk(to_addr)
				n_timeout = 0
			self.sock.sendto(msg,to_addr)
			if self.v:
				print 'Sent [' + msg + '] to',to_addr
			try:
				data,from_addr = self.sock.recvfrom(256)
			except Exception, e:
				n_timeout += 1
				print 'timeout!'
				continue
			if self.v:
				print 'Recv [' + data + '] from',from_addr
			if data == 'unknown_msg':
				raise Exception('Unknown message received')
			break
		return from_addr

	def tryWalk(self,to_addr):
		print 'TRYING WALK PROCEDURE'
		offset = 10
		work = False
		while offset <= 1000:
			# Set fso with offset
			self.fso.setWithOffset(offset)
			# ask if there, if no response continue
			self.sock.sendto('Anyone there?',to_addr)
			if self.v:
				print 'Sent [Anyone there?] to',to_addr
			try:
				data,from_addr = self.sock.recvfrom(256)
			except Exception, e:
				if offset > 0:
					offset = -offset
				else:
					offset = -offset + 10
				continue
			work = True
			if self.v:
				print 'Recv [' + data + '] from',from_addr
			break
		if work:
			sign = offset / abs(offset)
			for o in xrange(offset,sign * -10,sign * -10):
				self.fso.setWithOffset(o)
				for x in range(10):
					self.sock.sendto('Anyone there?',to_addr)
					if self.v:
						print 'Sent [Anyone there?] to',to_addr
					try:
						data,from_addr = self.sock.recvfrom(256)
					except Exception, e:
						continue
					if self.v:
						print 'Recv [' + data + '] from',from_addr
					break
				if x == 10:
					print "WALK PROCEDURE DIDN'T WORK"
					return
			print 'WALK PROCEDURE WORKED'
		else:
			print "WALK PROCEDURE DIDN'T WORK"
			
	def useFSO(self):
		return self.fso != None

def runTest(sender,msg_len,freq,norm_wt,test_time,config_file,out_file):
	start_test_msg = 'start_test msg_char=A msg_len=' + str(msg_len) + ' test=throughput switch_freq=' + str(freq)
	if norm_wt != None:
		start_test_msg += ' norm_wt=' + str(norm_wt)

	sender.sendBoth(start_test_msg)
	if sender.useFSO():
		sender.fso.gm.disconnectDevice()
		alt = subprocess.Popen(['../alternate/alternate','-input',config_file,'-f',str(freq)])

	info_str = 'msglen=' + str(msg_len)
	if sender.useFSO():
		info_str += ',freq=' + str(freq)
	if norm_wt != None:
		info_str += ',normwt=' + str(norm_wt)
	if angle >= 0:
		info_str += ',angle=' + str(angle)

	# Start receive
	recv = subprocess.Popen(['./receiver','-port',str(sender.test_port),'-info',info_str,'-out',out_file,'-packet',str(msg_len),'-kb','-mb'])
	time.sleep(test_time)

	# Stop througput test
	recv.send_signal(signal.SIGINT)
	recv.wait()

	if sender.useFSO():
		alt.terminate()
		alt.wait()

		sender.fso.gm.connectDevice()

	sender.sendBoth('end_test')

def runLatencyTest(sender,msg_len,norm_wt,num_test,lat_wt,config_file,out_file):
	char1 = 'A'
	char2 = 'B'

	start_test_msg1 = 'start_test msg_char=' + char1 + ' msg_len=' + str(msg_len) + ' test=latency'
	if norm_wt != None:
		start_test_msg1 += ' norm_wt=' + str(norm_wt)

	start_test_msg2 = 'start_test msg_char=' + char2 + ' msg_len=' + str(msg_len) + ' test=latency'
	if norm_wt != None:
		start_test_msg2 += ' norm_wt=' + str(norm_wt)

	sender.sendBoth(start_test_msg1,start_test_msg2)

	sender.fso.gm.disconnectDevice()

	# lat_proc = subprocess.Popen(['../latency/latency','-tp',str(sender.test_port),'-w',str(lat_wt),'-nt',str(num_test),'-ps',str(msg_len),'-out',out_file,'-config',config_file])
	lat_proc = subprocess.Popen(['python','../latency/latency.py','-tp',str(sender.test_port),'-w',str(lat_wt),'-nt',str(num_test),'-ps',str(msg_len),'-a',str(angle),'-out',out_file,'-config',config_file])

	lat_proc.wait()

	sender.fso.gm.connectDevice()

	sender.sendBoth('end_test')

def main(fso,end_experiment,local_port,foreign_port,test_port,timeout,verbose,num_test,msg_lens,freqs,norm_wts,test_time,config_file,out_file,latency,lat_wts):
	sock = socket(AF_INET,SOCK_DGRAM)
	sock.bind(('',local_port))
	sock.setsockopt(SOL_SOCKET,SO_BROADCAST,1)
	sock.settimeout(timeout)

	sender = Sender(fso,sock,foreign_port,test_port,verbose)

	if not sender.useFSO():
		freqs = [1]

	if latency:
		for ml in msg_lens:
			for n_wt in norm_wts:
				for lat_wt in lat_wts:
					runLatencyTest(sender,ml,n_wt,num_test,lat_wt,config_file,out_file)
	else:
		for ml in msg_lens:
			for f in freqs:
				for n_wt in norm_wts:
					for i in range(num_test):
						# Send messages to set up test
						runTest(sender,ml,f,n_wt,test_time,config_file,out_file)
	
	if end_experiment:
		sender.sendBoth('end_experiment')
	
	sock.close()

def getValues(fname):
	f = open(fname)
	vs = []
	for line in f:
		vs += map(float,line.split())
	return vs

def processParams(vs):
	result = []
	for v in vs:
		if v == None or v == 'None':
			result.append(None)
		elif v[-4:] == '.txt':
			result += getValues(v)
		else:
			result.append(float(v))
	return result

angle = -1

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Runs the receiver end of the througput test')

	# Controller parameters
	parser.add_argument('-lp','--local_port',metavar = 'LP',type = int,nargs = 1,default = [8887],help = 'Local port used to control test')
	parser.add_argument('-fp','--foreign_port',metavar = 'FP',type = int,nargs = 1,default = [8888],help = 'Foreign port used to control test')
	parser.add_argument('-tp','--test_port',metavar = 'TP',type = int,nargs = 1,default = [9898],help = 'Port used for test')
	parser.add_argument('-tout','--time_out',metavar = 'TO',type = float,nargs = 1,default = [.1],help = 'Timeout for control socket')
	parser.add_argument('-w','--wait_time',metavar = 'WT',type = float,nargs = 1,default = [.05],help = 'Time to wait between switching fso and sending message')
	
	parser.add_argument('-nt','--num_test',metavar = 'NT',type = int,nargs = 1,default = [1],help = 'Number of tests to run')
	parser.add_argument('-len','--test_length',metavar = 'LEN',type = int,nargs = 1,default = [5],help = 'Length of each test in seconds')
	parser.add_argument('-out','--out_file',metavar = "OFILE",type = str,nargs = 1,default = ['data.txt'],help = 'File to write data to')
	parser.add_argument('-config','--config_file',metavar = "CFILE",type = str,nargs = 1,default = ['align.config'],help = 'File with configuration of the network')
	
	parser.add_argument('-one','--one_host',action = 'store_true',help = 'If flag set then only one transmitter used')
	parser.add_argument('-v','--verbose',action = 'store_true',help = 'Use to specify verbose mode')
	parser.add_argument('-end','--end_experiment',action = 'store_true',help = 'If set then code on transmitters will end')
	
	
	# Parameters for both tests
	parser.add_argument('-lens','--msg_lens',metavar = 'LS',type = str,nargs = '+',default = ['1'],help = 'Values to use for message lengths')
	parser.add_argument('-norm','--norm_wait_time',metavar = 'N',type = str,nargs = '+',default = [None],help = 'Value to ad between messages for transmitter')
	
	# Throughput parameters
	parser.add_argument('-fs','--freqs',metavar = 'FS',type = str,nargs = '+',default = ['1'],help = 'Values to use for frequency')

	# Latency parameters
	parser.add_argument('-latency','--latency_test',action = 'store_true',help = 'If flag set then latency test will be run')
	parser.add_argument('-lat_wt','--latency_wait_time',metavar = 'LWT',type = str,nargs = '+',default = [],help = 'Wait time parameter for latency test')
	parser.add_argument('-lat_freq','--latency_frequency',metavar = 'LF',type = str,nargs = '+',default = [],help = 'Frequency for latency test')

	# Global parameters
	parser.add_argument('-a','--gm_angle',metavar = 'ANG',type = int,nargs = 1,default = [-1],help = 'Angle gm turns')
	

	args = parser.parse_args()

	angle = args.gm_angle[0]

	one_host = args.one_host
	v = args.verbose
	end_experiment = args.end_experiment

	lp = args.local_port[0]
	fp = args.foreign_port[0]
	tp = args.test_port[0]
	to = args.time_out[0]
	wt = args.wait_time[0]
	
	num_test = args.num_test[0]
	test_length = args.test_length[0]
	out_file = args.out_file[0]
	config_file = args.config_file[0]

	msg_lens = map(lambda x:2**int(x),processParams(args.msg_lens))
	freqs = map(lambda x:1/x,processParams(args.freqs))
	norm_wts = processParams(args.norm_wait_time)

	latency = args.latency_test
	lat_wts = processParams(args.latency_wait_time) + map(lambda x:1/x,processParams(args.latency_frequency))

	if not gm_supported and not one_host:
		print 'This machine does not have the GM library installed'
		one_host = True

	if one_host:
		fso = None
	else:
		fso = SimpleFSO(config_file,wt)

	if latency and len(lat_wts) == 0:
		lat_wts = [.1]

	if one_host and latency:
		print 'Cannot run latency test with only one transmitter'
	else:
		main(fso,end_experiment,lp,fp,tp,to,v,num_test,msg_lens,freqs,norm_wts,test_length,config_file,out_file,latency,lat_wts)
