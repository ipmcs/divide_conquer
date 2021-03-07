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
#baseline = 'v1_base'
baseline = 'v1'

for _, dirnames, filenames in os.walk(path):
  # ^ this idiom means "we won't be using this value"
    folders += len(dirnames)

#print(folders)


folders = folders 

count2 = 2
IPCBaseLine = {}
accuracy = []
MPKI = []

#LoadMissBaseLine = {}

for count, fpath in enumerate(sorted(os.listdir(path)), start=1):
	if(fpath.find(baseline) >= 0):
		continue

	print('fpath = ' + fpath)
	i = 1
	speedup = []
	f1 = open("predictor.txt", "w")

   	for count1, fpath1 in enumerate(sorted(os.listdir(path+'/'+fpath+'/')), start=1):
		#print(fpath1)
			
		f = open(path+'/'+fpath+'/'+fpath1)
	
	   	Index1 = fpath1.find('.champsimtrace')
	   	trace = fpath1[0:Index1]
	   	#print("trace: " + trace)
	   	#sheet.write(count1,0,trace)
	   	prefetcher = fpath
		f1.write("\n\n\n" + trace + "\n")	   	
	  
	
	   	#sheet.write(0,count,prefetcher)

		while True:
			Index = -1
			j = 1
			line = f.readline()
			if not line:
				break 

			Index = line.find('Branch Prediction Accuracy: ')
					#print('index')
					#print(Index)

			if Index >= 0:
				Index1 = line.find('%')
				Index2 = line.find('MPKI: ')
				Index3 = line.find('Average')

				acc = float(line[Index+len('Branch Prediction Accuracy: '):Index1])
				mpki = float(line[Index2+len('MPKI: '):Index3].rstrip())

				f1.write("\tacc: " + str(acc) + "\n")
				f1.write("\tMPKI: " + str(mpki) + "\n")

				accuracy.append(acc)
				MPKI.append(mpki)
				#sheet.write(count1,count,ipc)
										
		f.close()

	#workbook.close()
	
	temp = 0
	for i in range(0, len(accuracy)) :  
    		temp = temp + accuracy[i]  
	temp = temp / len(accuracy)

	temp1 = 0
	for i in range(0, len(MPKI)) :  
    		temp1 = temp1 + MPKI[i]  
	temp1 = temp1 / len(MPKI)
	

	f1.write("\n\naccuracy " + fpath + ' = ' + str(temp) + "\n")
	f1.write("MPKI: " + fpath + ' = ' + str(temp1) + "\n")
	f1.close()		
