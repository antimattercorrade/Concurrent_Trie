import matplotlib.pyplot as plt
import csv
import numpy as np
from scipy.interpolate import make_interp_spline, BSpline

def plot_graph():

    for i in range (3):
        x=[]
        y=[]
        # val = 0
        # total = 0

        if(i == 0):
            file = "./CSV/s_lock.csv"
        elif(i == 1):
            file = "./CSV/rw_lock.csv"
        elif(i == 2):
            file = "./CSV/hoh_lock.csv"

        with open(file, 'r') as csvfile:
            plots= csv.reader(csvfile, delimiter=',')
            for row in plots:
                # val = val+1
                # total += float(row[1])
                x.append(float(row[0]))
                # y.append(float(total)/float(val))
                y.append(float(row[1]))
        
        x = np.array(x)
        y = np.array(y)

        x_new = np.linspace(x.min(), x.max(), 1000) 
        spl = make_interp_spline(x, y, k=3)
        y_smooth = spl(x_new)

        if(i == 0):
            plt.plot(x_new,y_smooth, color="orange", label="Single Lock", linewidth=2)
        elif(i == 1):
            plt.plot(x_new,y_smooth, color="yellow", label="RW Lock", linewidth=2)
        elif(i == 2):
            plt.plot(x_new,y_smooth, color="green", label="HOH Lock", linewidth=2)

    plt.title("Performance")

    plt.xlabel('Concurrent Threads')
    plt.ylabel('Time (in sec)')
    plt.legend(loc='best')
    
    plt.savefig("Performance", dpi=450)
    plt.close()

plot_graph()