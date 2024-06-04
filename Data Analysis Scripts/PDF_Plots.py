import numpy as np
import matplotlib.pyplot as plt
import csv
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

# Data file
dataFileName = "test4.txt"

# PDF info
outputFileName = "test4.pdf"
title = "Dyno Test 4"
date = "21/03/2024"
driver = "--"
description = "Dyno Test 4"

# Plot Formatting
titleFontSize = 28
axisLabelFontSize = 18

totalColumns = 13
columnsWithoutGPS = 6
gpsData = False

# Open the CSV file
with open(dataFileName, 'r') as file:

    plotArray = []
    gpsArray = []    

    # skips header line (needs to be here if we add a header line)
    file.seek(0)
    file.readline()

    reader = csv.reader(file)

    # Iterate over each row in the CSV file
    for row in reader:
        row[:] = [x for x in row if x]
    
        if len(row) == columnsWithoutGPS:
            row = row + ["0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"]
            plotArray.append(row) # pad with 0s, so that power can be index 13, 14 etc.
        elif len(row) == totalColumns:
            row = row + ["0", "0", "0", "0"]
            gpsArray.append(row)
            plotArray.append(row)

    # convert to numpy arrays
    plotArray = np.array(plotArray).astype('float')
    gpsArray = np.array(gpsArray).astype('float')


plotArray = plotArray.transpose()
gpsArray = gpsArray.transpose()

# convert ms to secs
plotArray[0] = plotArray[0]/1000

# Overall power
plotArray[13] = plotArray[1] * plotArray[3] # overall power

# Motor power
plotArray[14] = plotArray[2] * plotArray[4]

# Percentage efficiency
plotArray[15] = plotArray[14]/plotArray[13]*100

#GPS Array Conversions
if gpsArray.size != 0:
    gpsData = True
    gpsArray[0] = gpsArray[0]/1000 # ms to secs
    gpsArray[12] = gpsArray[12] * 3.6 # m/s to km/h
    gpsArray[13] = gpsArray[1] * gpsArray[3] # Overall Power Consumption
    gpsArray[14] = gpsArray[2] * gpsArray[4] # Motor Power Consumption
    gpsArray[15] = gpsArray[14]/gpsArray[13]*100 # Percentage Efficiency


labels = { 0: "Time (s)",
           1: "Battery Current (A)",
           2: "Motor Current (A)",
           3: "Battery Voltage (V)",
           4: "Motor Voltage (V)",
           5: "Hall Sensor Speed (km/h)",
           6: "",
           7: "",
           8: "UTC Time",
           9: "Latitude",
           10: "Longitude",
           11: "Altitude",
           12: "GPS Speed (km/h)",
           13: "Power (W)",
           14: "Motor Power (W)",
           15: "Percentage Efficiency (%)"}


if gpsData:
    # Plot 1 (GPS Speed vs Time)
    fig1, ax = plt.subplots(figsize=(14, 12))
    ax.plot(gpsArray[0], gpsArray[12], lw=2)
    plt.title("GPS Speed vs. Time", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[12], fontsize=axisLabelFontSize, weight='bold')
    ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
    ax.minorticks_on()
    ax.tick_params(which = "minor", bottom = False, left = False)
    ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
    ax.tick_params(which = "major", bottom = False, left = False)

# Plot 2 (Battery Current vs Time)
fig2, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[1], lw=2)
plt.title("Battery Current vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[1], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 3 (Motor Current vs Time)
fig3, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[2], lw=2)
plt.title("Motor Current vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[2], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 4 (Battery Voltage vs Time)
fig4, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[3], lw=2)
plt.title("Battery Voltage vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[3], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 5 (Motor Voltage vs Time)
fig5, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[4], lw=2)
plt.title("Motor Voltage vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[4], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 6 (Overall Power Output vs Time)
fig6, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[13], lw=2)
plt.title("Overall Power Output vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[13], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 7 (Motor Power Output vs Time)
fig7, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[14], lw=2)
plt.title("Motor Power Output vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[14], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

if gpsData:
    # Plot 8 (GPS Speed vs Position)
    fig8, ax = plt.subplots(figsize=(14, 12))
    plt.title("GPS Speed vs Position", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[9], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[10], fontsize=axisLabelFontSize, weight='bold')
    cbar = plt.colorbar(ax.scatter(gpsArray[10], gpsArray[9], c=gpsArray[12]))
    cbar.set_label(labels[12], fontsize = axisLabelFontSize, weight='bold')

if gpsData:
    # Plot 9 (Battery Current vs Position)
    fig9, ax = plt.subplots(figsize=(14, 12))
    plt.title("Battery Current vs Position", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[9], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[10], fontsize=axisLabelFontSize, weight='bold')
    cbar = plt.colorbar(ax.scatter(gpsArray[10], gpsArray[9], c=gpsArray[1]))
    cbar.set_label(labels[1], fontsize = axisLabelFontSize, weight='bold')

if gpsData:
    # Plot 10 (Motor Voltage vs Position)
    fig10, ax = plt.subplots(figsize=(14, 12))
    plt.title("Motor Voltage vs Position", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[9], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[10], fontsize=axisLabelFontSize, weight='bold')
    cbar = plt.colorbar(ax.scatter(gpsArray[10], gpsArray[9], c=gpsArray[4]))
    cbar.set_label(labels[4], fontsize = axisLabelFontSize, weight='bold')

if gpsData:
    # Plot 11 (Overall power vs Position)
    fig11, ax = plt.subplots(figsize=(14, 12))
    plt.title("Overall Power vs Position", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[9], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[10], fontsize=axisLabelFontSize, weight='bold')
    cbar = plt.colorbar(ax.scatter(gpsArray[10], gpsArray[9], c=gpsArray[13]))
    cbar.set_label(labels[13], fontsize = axisLabelFontSize, weight='bold')

# Plot 12 (Motor Current vs. Battery Current)
fig12, ax = plt.subplots(figsize=(14, 12))
ax.scatter(plotArray[1], plotArray[2])
plt.title("Motor Current vs. Battery Current", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[1], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[2], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "major")
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 13 (Motor Power vs. Battery Power)
fig13, ax = plt.subplots(figsize=(14, 12))
ax.scatter(plotArray[13], plotArray[14])
plt.title("Motor Power vs. Battery Power", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[13], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[14], fontsize=axisLabelFontSize, weight='bold')
ax.grid(which = "major")
ax.tick_params(which = "major", bottom = False, left = False)

# Plot 14 (Percentage efficiency vs. Time)
fig14, ax = plt.subplots(figsize=(14, 12))
ax.plot(plotArray[0], plotArray[15], lw=2)
plt.title("Efficiency vs. Time", fontsize=titleFontSize, weight='bold')
plt.xlabel(labels[0], fontsize=axisLabelFontSize, weight='bold')
plt.ylabel(labels[15], fontsize=axisLabelFontSize, weight='bold')
ax.set(ylim=(-20, 150))
ax.grid(which = "minor", lw=0.1, color=[0.8, 0.8, 0.8])
ax.minorticks_on()
ax.tick_params(which = "minor", bottom = False, left = False)
ax.grid(which = "major", lw=1.5, color=[0.56, 0.56, 0.56])
ax.tick_params(which = "major", bottom = False, left = False)

if gpsData:
    # Plot 15 (Overall power vs Position)
    fig15, ax = plt.subplots(figsize=(14, 12))
    plt.title("Efficiency vs Position", fontsize=titleFontSize, weight='bold')
    plt.xlabel(labels[9], fontsize=axisLabelFontSize, weight='bold')
    plt.ylabel(labels[10], fontsize=axisLabelFontSize, weight='bold')
    cbar = plt.colorbar(ax.scatter(gpsArray[10], gpsArray[9], c=gpsArray[15]))
    cbar.set_label(labels[15], fontsize = axisLabelFontSize, weight='bold')


# Saves plots and title/description to a PDF
with PdfPages(outputFileName) as pdf:
    # Title Page
    firstPage = plt.figure()
    firstPage.clf()
    firstPage.text(0.05,0.9, title, transform=firstPage.transFigure, size=24, weight="bold", wrap=True, va='top')
    firstPage.text(0.05, 0.65, "Date: " + date, transform=firstPage.transFigure, size=18)
    firstPage.text(0.05, 0.55, "Driver: " + driver, transform=firstPage.transFigure, size=18)
    firstPage.text(0.05, 0.45, "Data File: " + dataFileName, transform=firstPage.transFigure, size=18)
    firstPage.text(0.05, 0.32, description, transform=firstPage.transFigure, size=14, wrap=True, va='top')
    pdf.savefig()

    # Plot Pages
    if gpsData:
        pdf.savefig(fig1)

    pdf.savefig(fig2)
    pdf.savefig(fig3)
    pdf.savefig(fig4)
    pdf.savefig(fig5)
    pdf.savefig(fig6)
    pdf.savefig(fig7)
    
    
    if gpsData:
        pdf.savefig(fig8)
        pdf.savefig(fig9)
        pdf.savefig(fig10)
        pdf.savefig(fig11)

    pdf.savefig(fig12)
    pdf.savefig(fig13)
    pdf.savefig(fig14)

    if gpsData:
        pdf.savefig(fig15)


# Show the saved PDF file path
print(f"Plots saved to {outputFileName}")