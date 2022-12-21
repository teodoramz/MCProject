from PyQt5 import QtWidgets, QtCore
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import sys  # We need sys so that we can pass argv to QApplication
import os
from random import randint
import serial

SerObj = serial.Serial('COM12')
class MainWindow(QtWidgets.QMainWindow):

    SerObj.baudrate = 115200

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
                    #tot ce tine de initizalizare grafic vine aici
        self.graphWidget = pg.PlotWidget()
        self.setCentralWidget(self.graphWidget)
        self.graphWidget.setBackground('w')#culoare fundal alb, pot sa il dau in hex, RGB sau cu litere asa
        penSoundVerde = pg.mkPen(color=(0, 255, 0))#line de culoare verde
        penRotAlbastru = pg.mkPen(color=(0, 0, 255))#linie de culoareAlbastra
        self.graphWidget.setTitle("Grafic cu valorile sunetului")
         #legenda axe
        styles = {'color':'k', 'font-size':'10px'}
        self.graphWidget.setLabel('left', 'Sound Values', **styles)
        self.graphWidget.setLabel('bottom', 'Time', **styles)
        #grid fundal(liniute pe ambele axe)
        self.graphWidget.showGrid(x=True, y=True)
        
        #limite axe
        self.graphWidget.setYRange(-20, 150, padding=0)#minim,maxim si padding
        
        
        self.sound = 0
        self.rot = 0
        self.lastValue = 0
        
        self.x = list(range(200))  # 200 time points
        self.y = [randint(0,1) for _ in range(200)]  # 200 data points
        self.z=[randint(0,1) for _ in range(200)]
        self.data_line =  self.graphWidget.plot(self.x, self.y,name="Sound", pen=penSoundVerde)
        self.data_line1= self.graphWidget.plot(self.x, self.z,name="Rot", pen=penRotAlbastru)
        self.timer = QtCore.QTimer()
        self.timer.setInterval(1)
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()
    
    def update_plot_data(self):

        self.x = self.x[1:]  # Remove the first y element.
        self.x.append(self.x[-1] + 1)  # Add a new value 1 higher than the last.

        for cv in range(25):
             ReceivedString = SerObj.readline()
        
        str=ReceivedString.decode('utf',errors='replace').strip("\n").strip("\r")
        value = str.split(":")
        try:
            if value[0] == "ROT":
                self.rot = int(value[1])
                if self.rot < self.lastValue - 1000 or self.rot > self.lastValue + 1000 or self.lastValue < 1000:
                    self.lastValue = self.rot
                    self.rot = int((self.rot*120)/65535)
                else:
                    self.rot = self.lastValue
                    self.rot =  int((self.rot*120)/65535)
            if value[0] == "SND":
                self.sound = int(value[1])
            
        except:
            #nothing
            a = 0
        finally:
        # rot = int((rot*120)/65535);
            print(self.rot, self.sound)
            self.y = self.y[1:]  # Remove the first
            self.z= self.z[1:]
            self.y.append(self.sound) 
            self.z.append(self.rot)
            #sound intre 0 40 verde 
            #40 80 galben
            #
        self.data_line.setData(self.x, self.y)  # Update the data.
        self.data_line1.setData(self.x, self.z)
app = QtWidgets.QApplication(sys.argv)
w = MainWindow()
w.show()
#sys.exit()
app.exec_()
