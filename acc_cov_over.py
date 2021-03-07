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
baseline='v1'

for _, dirnames, filenames in os.walk(path):
  # ^ this idiom means "we won't be using this value"
    folders += len(dirnames)

#print(folders)


folders = folders 

count2 = 2
MissBaseLine = {}
coverage = []
overprediction = []
accuracy = []
#LoadMissBaseLine = {}


for fname in os.listdir('./experiments/' + baseline + '/'):
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

			index1 = line.find('L1I TOTAL')
			
			if index1 > -1:
				Index = line.find('MISS:')
				miss = line[Index+5:].lstrip()
				#print 'miss baseline ' + tracename + ': ' + str(miss)
				MissBaseLine[tracename] = int(miss)


for count, fpath in enumerate(sorted(os.listdir(path)), start=1):
	print('fpath = ' + fpath)
	if(fpath.find(baseline) >= 0):
		continue

	i = 1
	coverage = []
	overprediction = []
	accuracy = []

   	for count1, fpath1 in enumerate(sorted(os.listdir(path+'/'+fpath+'/')), start=1):
		f = open(path+'/'+fpath+'/'+fpath1)
		f1 = open("acc_cov_over.txt", "a")

	   	Index1 = fpath1.find('.champsimtrace')
	   	trace = fpath1[0:Index1]
	   	prefetcher = fpath
	
		f1.write("\n\n\n" + trace + "\n")

		while True:
			Index = -1
			j = 1
			line = f.readline()
			if not line:
				break 

			Index = line.find('L1I LOAD')

			if Index >= 0:
				Index = line.find('MISS:')
				miss = int(line[Index+5:].lstrip())
				#print 'miss load prefetch ' + trace + ': ' + str(miss)
				cvr = 1 - float(miss) / MissBaseLine[trace]
				f1.write('\tload miss: ' + str(miss) + '\tbaseline miss: ' + str(MissBaseLine[trace]) + "\n")
				f1.write('\tcoverage: ' + str(cvr) + "\n")
				coverage.append(cvr)

			Index = line.find('L1I PREFETCH  REQUESTED:')

			if Index >= 0:
				Index = line.find('USELESS:')
				#print line
				useless = int(line[Index+8:].lstrip())
				#print 'useless load prefetch ' + trace + ': ' + str(useless)
				ovr = float(useless) / MissBaseLine[trace]
				f1.write('\tuseless: ' + str(useless) + '\n')
				f1.write('\toverprediction: ' + str(ovr) + "\n")
				overprediction.append(ovr)

				Index1 = line.find('USEFUL:')
				useful = int((line[Index1+7:Index].lstrip()).rstrip())
				#print 'useful ' + trace + ': ' + str(useful)
				acc = float(useful)/(useful+useless)
				f1.write('\tuseful: ' + str(useful) + "\n")
				f1.write('\taccuracy: ' + str(acc) + "\n")
				accuracy.append(acc)
						
		f.close()
	
	temp = 0
	for i in range(0, len(accuracy)) :  
    		temp = temp + accuracy[i]  

	avrg_acc = temp/len(accuracy)

	temp = 0
	for i in range(0, len(overprediction)) :  
    		temp = temp + overprediction[i]  

	avrg_over = temp/len(overprediction)

	temp = 0
	for i in range(0, len(coverage)) :  
    		temp = temp + coverage[i]  

	avrg_cvr = temp/len(coverage)

	f1.write("\n\n\ncoverage " + fpath + ' = ' + str(avrg_cvr) + "\n")
	f1.write("overprediction " + fpath + ' = ' + str(avrg_over) + "\n")
	f1.write("accuracy " + fpath + ' = ' + str(avrg_acc) + "\n")

	print "coverage " + fpath + ' = ' + str(avrg_cvr) + "\n"
	print "overprediction " + fpath + ' = ' + str(avrg_over) + "\n"
	print "accuracy " + fpath + ' = ' + str(avrg_acc) + "\n"

	f1.close()		
