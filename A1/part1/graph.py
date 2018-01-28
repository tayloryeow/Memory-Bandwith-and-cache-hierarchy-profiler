import random

import leather

#Parse benchmark data into dat_data

test="benchmark"

i = 0;
f = open('data/'+test, 'r')
dot_data = []
for line in f:
    dot_data.append((int(i), (map(int, line.split(' '))[1])))
    print dot_data[-1]
    i+= 1
print dot_data
f.close()

chart = leather.Chart('Charts: '+test)
chart.add_line(dot_data)
chart.to_svg('examples/charts/'+ test +'.svg')


