
def add_angle(filename,angle):
	# Read in file
	f = open(filename)
	lines = []
	for line in f:
		# Add angle header
		spl = line.split()
		if '=' not in spl[0]:
			hs = spl[0].split('_')
			hs = [hs[i] + '=' + hs[i + 1] for i in xrange(0,len(hs),2)]
			spl[0] = ','.join(hs)
		spl[0] += ',angle=' + str(angle)
		lines.append('\t'.join(spl) + '\n')
	
	f.close()

	# Write file
	f = open(filename,'w')
	for line in lines:
		f.write(line)

files = ['latency_data.txt', 'norm_data.txt','norm_test.txt','test.txt','video_data.txt']
angle = 14.7

for f in files:
	add_angle(f,angle)
