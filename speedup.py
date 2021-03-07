#!/usr/bin/env python

import os
import glob
import math
#import numpy as np
#import xlwt 
#from xlwt import Workbook 

#import xlsxwriter
#workbook   = xlsxwriter.Workbook('./result_testing.xlsx')

#sheet = workbook.add_worksheet()


#sheet.write('A1', 'Trace')


i  = 1
# Let's create a dictionary, the functional way 

# Create your dictionary class 
folders = 0
path = "./experiments/"

for _, dirnames, filenames in os.walk(path):
  # ^ this idiom means "we won't be using this value"
    folders += len(dirnames)

#print(folders)


folders = folders 

count2 = 2
IPCBaseLine = {}
speedup = []
#LoadMissBaseLine = {}
baseline='v1'

for fname in os.listdir('./experiments/'+baseline+'/'):
#if True:
	fpath = './experiments/' + baseline + '/' + fname
	#print(fpath)
	#fname = 'server_036.champsimtrace.xz'
	#fpath = './results_50M/server_036.champsimtrace.xz-hashed_perceptron-tablePIF-next_line-spp_dev-no-lru-1core_pif_table.txt'
	if os.path.isfile(fpath):

		f = open(fpath)	
		Index5 = (fpath.find('/'))
		Index6 = (fpath.find('/',Index5+2))
		Index7 = (fpath.find('/',Index6+2))
		Index6_2 = fpath.find('.champsimtrace')
		tracename = fpath[Index7+1:Index6_2]
		#print(tracename)
		while True:
			Index = -1
			j = 1
			line = f.readline()
			if not line:
				break 

			index1 = line.find('Interest')
			
			if index1 > -1:
				#print(index1)
				line = f.readline()
				line = f.readline()
				Index = line.find('IPC')
				Index2 = line.find('instructions')
				ipc = float(line[Index+5:Index2-1])
				IPCBaseLine[tracename] = ipc


for count, fpath in enumerate(sorted(os.listdir(path)), start=1):
	print('fpath = ' + fpath)
	if(fpath.find(baseline) >= 0):
		continue;
	i = 1
	speedup = []
	f1 = open("speedup.txt", "w")

   	for count1, fpath1 in enumerate(sorted(os.listdir(path+'/'+fpath+'/')), start=1):
		#print(fpath1)
			
		f = open(path+'/'+fpath+'/'+fpath1)

	   	Index1 = fpath1.find('.champsimtrace')
	   	trace = fpath1[0:Index1]
	   	#print("trace: " + trace)
	   	#sheet.write(count1,0,trace)
	   	prefetcher = fpath
	   	
	  	f1.write("\n\n" + trace + "\n")
	
	   	#sheet.write(0,count,prefetcher)

		while True:
			Index = -1
			j = 1
			line = f.readline()
			if not line:
				break 

			Index = line.find('Interest')
					#print('index')
					#print(Index)

			if Index > 1:
				line = f.readline()
				line = f.readline()
				Index = line.find('IPC')
				Index2 = line.find('instructions')
				ipc = float(line[Index+5:Index2-1])
						#print(ipc1)
				#sheet1.write(i, j, ipc1)
						#j = j + 1
				#print 'ipc = ' + str(ipc) + ' base = ' + str(IPCBaseLine[trace])

				ipc = ipc/IPCBaseLine[trace]
				#print 'speedup = ' + str(ipc)
				f1.write("\t" + str(ipc) + "\n")
				speedup.append(ipc)
				#sheet.write(count1,count,ipc)
										
		f.close()
	#workbook.close()
	#print "len speedup = " + str(len(speedup))
	#print speedup
	# Geometric Mean of List  
	# using loop + formula  
	temp = 1
	for i in range(0, len(speedup)) :  
    		temp = temp * speedup[i]  
		#print 'temp = ' + str(temp)

	temp2 = (float)(math.pow(temp, (1.0/ len(speedup))))
	res = (float)(temp2)  
	#print 'geomean = ' + str(res)
	f1.write("\n\nspeedup " + fpath + ' = ' + str(res) + "\n")
	print 'speedup: ' + fpath + ' = ' + str(res) + '\n'
	f1.close()		
