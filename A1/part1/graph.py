import random
import argparse

import leather

#Parse benchmark data into dat_data
def get_random_color(pastel_factor = 0.5):
    return [(x+pastel_factor)/(1.0+pastel_factor) for x in [random.uniform(0,1.0) for i in [1,2,3]]]

def color_distance(c1,c2):
    return sum([abs(x[0]-x[1]) for x in zip(c1,c2)])

def generate_new_color(existing_colors,pastel_factor = 0.5):
    max_distance = None
    best_color = None
    for i in range(0,100):
        color = get_random_color(pastel_factor = pastel_factor)
        if not existing_colors:
            return color
        best_distance = min([color_distance(color,c) for c in existing_colors])
        if not max_distance or best_distance > max_distance:
            max_distance = best_distance
            best_color = color
def double_to_hex(f):
    return hex(struct.unpack('<Q', struct.pack('<d', f))[0])


parser = argparse.ArgumentParser(description='Plot testing data')
parser.add_argument('testName', type=str, help='Name of the Test File')
print "after parser"

args = parser.parse_args()

writeSize = {}
f = open('data/'+ args.testName, 'r')
for line in f:
    line = line.split(' ')
    dataSize = int(line[2])
    
    #First Occurancee
    if not writeSize.has_key(dataSize):
        writeSize[dataSize] = []
        i = 0

    writeSize[dataSize].append((int(i), int(line[3])))
    print writeSize[dataSize][-1]
    i+= 1
f.close()

#Setup Margins, labels, and Name
chart = leather.Chart('Charts: '+ args.testName)
chart.add_y_axis(name="nanoseconds")
chart.add_x_axis(name='Sample Number')

chart.add_line(writeSize)

print 'testName' + args.testName
'''

#Write out chart to file
print args.testName

chart.to_svg('examples/charts/'+ args.testName + '.svg')

print "Printed file to " + "examples/charts" + args.testName + ".svg"

'''
