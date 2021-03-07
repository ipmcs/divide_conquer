#!/usr/bin/env python

import os
import glob
#import xlwt 
#from xlwt import Workbook 

import xlsxwriter
workbook   = xlsxwriter.Workbook('./coverage_overprediction.xlsx')

sheet = workbook.add_worksheet()


sheet.write('A1', 'Trace')
sheet.write('C1', 'COVERAGE')
sheet.write('D1', 'NONCOVERAGE')
sheet.write('E1', 'UNTIME')
sheet.write('F1', 'OVER')


f = 1
i  = 1
# Let's create a dictionary, the functional way 

# Create your dictionary class 
files = folders = 0
path = "./coverage/"

for _, dirnames, filenames in os.walk(path):
  # ^ this idiom means "we won't be using this value"
    folders += len(dirnames)

#print(folders)


folders = folders - 1

count2 = 2
TotalMissBaseLine = {}
LoadMissBaseLine = {}


for fname in os.listdir('./coverage/v447/'):
#if True:
	fpath = './coverage/v447/' + fname
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
				#print(index1)
				indexendline1 = line.rfind('\n')
						#print(indexendline)
				indexmissload = line.find('MISS')
				misload1 = line[indexmissload+5:indexendline1].lstrip()
				misload = misload1.rstrip()
				missload = int(misload)
				
				#print(missload)
						#print(line[indexmissload+5:indexendline1])
							#print(mistotal)
				TotalMissBaseLine[tracename] = missload
				line = f.readline()
				Index2 = line.find('L1I LOAD')
			#print('index')
			#print(Index)
				
				if Index2 > -1:
					#print(Index2)
					indexendline1 = line.rfind('\n')
							#print(indexendline)
					indexmissload = line.find('MISS')
					misload1 = line[indexmissload+5:indexendline1].lstrip()
					misload = misload1.rstrip()
					missload = float(misload)
#					print(missload)
							#print(line[indexmissload+5:indexendline1])
								#print(mistotal)
					LoadMissBaseLine[tracename] = missload



	#for fname in os.listdir('./results_50M/'+folder_name[x]):
prefetcher = 'a'
L1  = 0
count1  =1
for count, fpath in enumerate(sorted(os.listdir(path)), start=1):
	
	i = 1
	k = 0
	a = 0
	ave_cov = 0
	ave_noncov = 0
	ave_untime = 0
	ave_over = 0
	
   	for count1, fpath1 in enumerate(sorted(os.listdir(path+'/'+fpath+'/')), start=1):
#		print(count1)
		L1 = 0
		if fpath == 'v1':
			a = 1
		else:	
			f = open(path+'/'+fpath+'/'+fpath1)

	   		Index1 = fpath1.find('.champsimtrace')
	   		trace = fpath1[0:Index1]
	   		print(trace)

	   		#print(trace)
	   		if count == 2:
		   		sheet.merge_range(i+count-2, 0, i+count-3+folders, 0, trace)
		   		#i = i + 4
			prefetcher = fpath
			print(prefetcher)
         	
		   	sheet.write(i+count-2,1,prefetcher)
            #print(prefetcher)
		   	while True:
					Index = -1
					j = 1
					line = f.readline()
					if not line:
						break 

					Index = line.find('L1I TOTAL')
					#print('index')
		#			print(Index)

					if Index > -1:
								indexendline1 = line.rfind('\n')
                                #print('index :')
							#	print(indexendline1)
								indexmissload = line.find('MISS')
								misload1 = line[indexmissload+5:indexendline1].lstrip()
								misload = misload1.rstrip()
								misstotal = int(misload)
					
								#print(line[indexmisstotal+5:indexendline])
								#print(mistotal)
								#sheet1.write(i, j, misstotal)
								
								line = f.readline()
								Index2 = line.find('L1I LOAD')
							#print('index')
							#print(Index)
								
								if Index2 > -1:
										#print(Index2)
										indexendline1 = line.rfind('\n')
												#print(indexendline)
										indexmissload = line.find('MISS')
										misload1 = line[indexmissload+5:indexendline1].lstrip()
										misload = misload1.rstrip()
										missload = float(misload)
										#print(line[indexmissload+5:indexendline1])
										#print(mistotal)
										#sheet1.write(i, j, missload)
		
										
										while True:
											line = f.readline()
											if not line:
												break	
											index_l1i = line.find('L1I')
											if index_l1i > -1:
												while True:
													line = f.readline()
													if not line:
														break
													index_utimely = line.find('utimely')
													if index_utimely > -1:
														indexendline1 = line.rfind('\n')
														
														untimely = line[index_utimely+len('utimely: '):indexendline1]
														utimely = float(untimely)
													
														
													#sheet1.write(i, j, miss_pf_PIF_useless)
														while True:
									
															line = f.readline()
															if not line:
																break	
															if L1 > 0:
																break
															index_utimely = line.find('useless:')
															if index_utimely > -1:
																	indexendline1 = line.rfind('\n')
																	#print(indexmiss_pf_PIF_useless)
																	useles = line[index_utimely+len('useless: '):indexendline1]
																	print('useles')
																	print(useles)
																	useless = float(useles)
																	L1 = 1
																	
																	coverage = float(float(1-float(missload/LoadMissBaseLine[trace]))*100)
																	ave_cov = ave_cov + coverage
																	#print(missload/LoadMissBaseLine[trace])
																	#print(missload)
																	#print(LoadMissBaseLine[trace])
																	#print(coverage)
																	#print(LoadMissBaseLine[trace])
																	#print(coverage)
																	sheet.write(i+count-2,2,coverage)
																	#miss_non_prefetch = 0
																	#miss_non_prefetch = missload - utimely
																	noncov = float(float((missload - utimely)/LoadMissBaseLine[trace])*100)
																	ave_noncov = ave_noncov + noncov
																	sheet.write(i+count-2,3,noncov)
																	untime = float(float(utimely/LoadMissBaseLine[trace])*100)
																	#print(untime)
																	ave_untime = ave_untime +untime
																	sheet.write(i+count-2,4,untime)
																	overpred = float(float(useless/TotalMissBaseLine[trace])*100)
																	sheet.write(i+count-2,5,overpred)
																	ave_over = ave_over + overpred
																	#print(overpred)
																	i = i + folders 
																	k = k + 1



				#sheet.cell(row = count2, column = 1).value = trace
			#sheet.cell(row = count2, column = 2).value = fpath

	   		#MissLoadBase = key_list[val_list.index(trace)]


		#if True:

			#if os.path.isdir(os.path.join('./results_50M/'+folder_name[x],fname)):
				#print(fname)
		#	fpath = './results_50M/'+folder_name[x] + fname
			
			#fname = 'server_036.champsimtrace.xz'

												#j = j + 1
										
		f.close()
	if a == 0:
#		print(count1)
		t = ( folders * count1)+1
		#if f == 1:
		#	print(f)
		sheet.merge_range(t , 0, t + 4, 0, 'Average')	
		#	f = 2
		sheet.write(t +count - 2,1,prefetcher)	
		ave_noncov = float(ave_noncov/count1)
		ave_untime = float(ave_untime/count1)
		ave_over = float(ave_over/count1)
		ave_cov = float(ave_cov/count1)
		sheet.write(t+count-2 ,2,ave_cov)
		sheet.write(t+count-2 ,3,ave_noncov)
		sheet.write(t+count-2 ,4,ave_untime)
		sheet.write(t+count-2 ,5,ave_over)

		#print(k)

workbook.close()
