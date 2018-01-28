import random
import argparse

import leather

#Parse benchmark data into dat_data

parser = argparse.ArgumentParser(description='Plot testing data')
parser.add_argument('testName', type=str, help='Name of the Test File')

args = parser.parse_args()

print args.testName
i = 0;
f = open('data/'+ args.testName, 'r')
dot_data = []
for line in f:
    dot_data.append((int(i), (map(int, line.split(' '))[3])))
    print dot_data[-1]
    i+= 1
print dot_data
f.close()

chart = leather.Chart('Charts: '+ args.testName)
chart.add_line(dot_data)
chart.to_svg('examples/charts/'+ args.testName +'.svg')
print "Printed file to " + "examples/charts" + args.testName + ".svg"

