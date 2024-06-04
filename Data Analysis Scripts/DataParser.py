# Script to Create a plot from the GEEC DAQ Data
# Authors: Conor Farrell & Breandán Gillanders

# Instructions: Change filename to desired txt file
#               Change 'xdata' variable to number corresponding to desired X-axis data (as detailed above the variable)
#               Change 'ydata' variable to number corresponding to desired Y-axis data
#               Change 'title' variable to desired plot title
#               Comment/Uncomment desired plot type (line plot/scatter plot)
#               If scatter plot is being used, colour bar data can also be set (uncomment colour bar code)

import numpy as np
import matplotlib.pyplot as plt
import csv
from PIL import Image

# SET FILENAME HERE *************************************************************************************************
# Python file should be in the same folder as the text file
filename = "5Laps.txt"
columns = 13

# Open the CSV file
with open(filename, 'r') as file:
    # Create a CSV reader object
    header = file.readline().strip('\n').replace(" ", "").split(',')
    count = sum(1 for line in file)

    print(count)
    arr = np.empty([count, columns])
    # arr[0] = header
    file.seek(0)
    file.readline()
    # print(arr)
    reader = csv.reader(file)

    # next(reader)
    # Iterate over each row in the CSV file
    for row in reader:
        row[:] = [x for x in row if x]
        jj = reader.line_num - 1
        # print(jj)
        innerArr = []
        for ii in range(columns):
            if ii < len(row):
                innerArr.append(row[ii])
            else:
                innerArr.append(0)

        arr[jj] = innerArr

npArr = np.array(arr).transpose()
npArr[0] = npArr[0] / 1000
npArr[12] = npArr[12] * 3.6
npArr[3] = npArr[3] / 1000
npArr[4] = npArr[4] / 1000

labels = { 0: "Time (s)",
           1: "Battery Current (A)",
           2: "Motor Current (V)",
           3: "Battery Voltage (V)",
           4: "Motor Voltage (V)",
           5: "Hall Sensor Speed (km/h)",
           6: "",
           7: "",
           8: "UTC Time",
           9: "Latitude",
           10: "Longitude",
           11: "Altitude",
           12: "GPS Speed (km/h)" }

# Data Analysis
fig, ax = plt.subplots(figsize=(14, 12))

# The NP array has the following columns:
# 0,       1,  2,  3,  4,          5,          6,       7,  8,         9,         10, 11,    12
# millis, iB, iM, vB, vM, Hall Speed, GPS Status, GPS Fix, UTC, latitude, longitude, ALT, SPEED

# SET DATA HERE *********************************************************************************************
xdata = 0
ydata = 12
cBarData = 12
title = ""

xlabel = labels[xdata]
ylabel = labels[ydata]
cbarLabel = labels[cBarData]

# SET PLOT TYPE HERE *******************************************************************************************
ax.plot(npArr[xdata], npArr[ydata])
# ax.scatter(npArr[xdata], npArr[ydata])

plt.title(title)
plt.xlabel(xlabel)
plt.ylabel(ylabel)

# COLOUR SCATTER PLOT ****************************************************************************************
# cbar = plt.colorbar(ax.scatter(npArr[xdata], npArr[ydata], c=npArr[cBarData]))
# cbar.set_label(cbarLabel)

plt.show()
