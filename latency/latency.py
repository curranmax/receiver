
from gm import GM
import argparse
import time
from socket import *
from datetime import datetime

class SimpleFSO:
	def __init__(self,config_file):
		self.val1 = None
		self.val2 = None
		self.is_val1 = True
		self.readConfig(config_file)
		self.gm = GM()
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

	def switchLink(self):
		if self.is_val1:
			self.gm.setValue(self.val2)
		else:
			self.gm.setValue(self.val1)
		self.is_val1 = not self.is_val1

def runTests(port,c1,c2,wait_time,num_test,packet_size,angle,out_file,config_file):
	sock = socket(AF_INET,SOCK_DGRAM)
	sock.bind(('',port))

	fso = SimpleFSO(config_file)

	vals = []
	for x in range(num_test):
		time.sleep(wait_time)

		data,addr = sock.recvfrom(packet_size + 100)
		if data[0] == c1:
			first_c1 = True
		else:
			first_c1 = False

		start_time = datetime.now()

		fso.switchLink()
		while True:
			data,addr = sock.recvfrom(packet_size + 100)
			if (data[0] == c1 and (not first_c1)) or (data[0] == c2 and first_c1):
				break

		end_time = datetime.now()

		vals.append((end_time - start_time).total_seconds())
	f = open(out_file,'a')

	f.write('packet_size=' + str(packet_size) + ',wait_time=' + str(wait_time) + ',language=python,angle=' + str(angle) + ' ')
	f.write(' '.join(map(str,vals)))
	f.write('\n')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Runs the receiver end of the througput test')

	parser.add_argument('-tp','--test_port',metavar = 'TP',type = int,nargs = 1,default = [9999],help = 'Local port used for test')
	parser.add_argument('-w','--wait_time',metavar = 'WT',type = float,nargs = 1,default = [1.0],help = 'time between tests')
	parser.add_argument('-nt','--num_test',metavar = 'NT',type = int,nargs = 1,default = [1],help = 'Number of tests to run')
	parser.add_argument('-ps','--packet_size',metavar = 'PS',type = int,nargs = 1,default = [1],help = 'Size of packet to be recieved')
	parser.add_argument('-out','--out_file',metavar = 'OF',type = str,nargs = 1,default = [''],help = 'File to output data')
	parser.add_argument('-config','--config_file',metavar = 'CF',type = str,nargs = 1,default = [''],help = 'File that has value which configure network')
	
	parser.add_argument('-c1','--char1',metavar = 'C1',type = str,nargs = 1,default = ['A'],help = 'Character from endpoint 1')
	parser.add_argument('-c2','--char2',metavar = 'C2',type = str,nargs = 1,default = ['A'],help = 'Character from endpoint 2')

	parser.add_argument('-a','--gm_angle',metavar = 'TP',type = int,nargs = 1,default = [-1],help = 'Angle GM rotates')
	

	args = parser.parse_args()

	angle = args.gm_angle[0]

	port = args.test_port[0]
	wait_time = args.wait_time[0]
	num_test = args.num_test[0]
	packet_size = args.packet_size[0]
	out_file = args.out_file[0]
	config_file = args.config_file[0]

	c1 = args.char1[0]
	c2 = args.char2[0]

	runTests(port,c1,c2,wait_time,num_test,packet_size,angle,out_file,config_file)
