#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import glob
import math

# import numpy as np
# import xlwt
# from xlwt import Workbook

import xlsxwriter
workbook = xlsxwriter.Workbook('result_testing.xlsx')

sheets = []
sheets_starting_idx = []

# sheet = workbook.add_worksheet()

# sheet.write('A1', 'Trace')

i = 1

# Let's create a dictionary, the functional way

# Create your dictionary class

folders = 0
path = "./experiments"

for (_, dirnames, filenames) in os.walk(path):

  # ^ this idiom means "we won't be using this value"

    folders += len(dirnames)

print (folders)

folders = folders

for (count, fpath) in enumerate(sorted(os.listdir(path)), start=1):

    # print('fpath = ' + fpath)
    # if(fpath.find('v1_base') >= 0):
    # ....continue;

    print ("new folder")

    for (count1, fpath1) in enumerate(sorted(os.listdir(path + '/'
            + fpath + '/')), start=1):
        traces_count =  len(os.listdir(path + '/' + fpath + '/'))
        print ("trace count: " + str(traces_count))
        f = open(os.path.join(path, fpath, fpath1))

        Index1 = fpath1.find('.champsimtrace')
        if Index1 >= 0:
            trace = fpath1[0:Index1]
        else:
            continue

        experiment_name = fpath

        print ('trace: ' + trace + ' experiment: ' + experiment_name)

           # sheet.write(0,count,prefetcher)

        sheet_count = -1
        experiment_name_col = 1
        attribute_name_col = 1

        while True:
            line = f.readline()
            if not line:
                break

            a = False            
            if line.find('SHEET') >= 0:
                a = True
                print ('NEW SHEET\n')
                print ('sheet count: ' + str(sheet_count))

                sheet_count += 1

                if count == 1 and count1 == 1:
                    print ("sheet created")
                    sheets.append(workbook.add_worksheet())
                    sheets_starting_idx.append(1)
                
                print("starting idx: " + str(sheets_starting_idx[sheet_count]))
                experiment_name_col = sheets_starting_idx[sheet_count]
                attribute_name_col =  sheets_starting_idx[sheet_count]

                items = 0
                type_1 = True
                name = ''

                while True:
                    line = f.readline()
                    if line.find(':') < 0:
                        if type_1:
                            sheets[sheet_count].write(count1 + 2, 0, trace)
                        else:
                            sheets[sheet_count].write(count1 + 1, 0, trace)

                        if count1 == 1: 
                            print ("writing experiment name from: " + str(experiment_name_col) + " to: " + str(experiment_name_col + items - 1)) 

                            if items != 1:
                                sheets[sheet_count].merge_range(0,
                                    experiment_name_col, 0,
                                    experiment_name_col + items - 1,
                                    experiment_name)

                                if(type_1):
                                    sheets[sheet_count].merge_range(1,
                                    experiment_name_col, 1,
                                    experiment_name_col + items - 1,
                                    name)
                            else:
                                sheets[sheet_count].write(0, experiment_name_col, experiment_name)

                        if count1 == traces_count:
                            sheets_starting_idx[sheet_count] = experiment_name_col + items
                            print ("starting idx updated to: " + str(sheets_starting_idx[sheet_count]))

                        break

                    if line.find(',') >= 0:
                        index1 = line.find(',')
                        index2 = line.find(':')
                        name = line[:index1]
                        attr = line[index1 + 1:index2]
                        value = line[index2 + 1:].strip()
                        if(value.find('nan') < 0):
                            value = float(value)


                        sheets[sheet_count].write(2, attribute_name_col, attr)
                        sheets[sheet_count].write(count1 + 2, attribute_name_col, value)

                        print ("writing att: " + str(attr) + " to: (2, " + str(attribute_name_col) + ")")
                        print ("writing value: " + str(value) + " to: (" + str(count1+2) + ", " + str(attribute_name_col) + ")")
                        
                        attribute_name_col += 1

                        #print ('n: ' + name + ' t: ' + attr + ' v: ' + value)

                        items += 1
                    else:
                        type_1 = False
                        index1 = line.find(':')
                        attr = line[:index1]
                        #print ("line: " + line)
                        value = line[index1 + 1:].strip()
                        if(value.find('nan') < 0):
                            value = float(value)

                        sheets[sheet_count].write(1, attribute_name_col, attr)
                        sheets[sheet_count].write(count1 + 1, attribute_name_col, value)

                        print ("writing att: " + str(attr) + " to: (1, " + str(attribute_name_col) + ")")
                        print ("writing value: " + str(value) + " to: (" + str(count1+1) + ", " + str(attribute_name_col) + ")")


                        attribute_name_col += 1

                        #print ('t: ' + attr + ' v: ' + value)

                        items += 1
            #if a:
            #    break
        f.close()

workbook.close()

