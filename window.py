import io
import sys
from serial.tools import list_ports
import serial
import folium
from PyQt5 import QtCore, QtGui, QtWidgets, QtWebEngineWidgets


class Window(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowIcon(QtGui.QIcon('gui_images/main_logo.png'))
        self.initWindow()


    def initWindow(self):
        self.setWindowTitle(self.tr("ELEN 509 - Final Project"))
        self.setFixedSize(1500, 800)
        self.portUI()
        self.remoteUI()
        self.buttonUI()
        self.ser = serial.Serial()
        #self.displayUI()


    def portUI(self):
        
        #Button to Update
        COM_UpdatePort_button = QtWidgets.QPushButton(self.tr("Update"))
        COM_UpdatePort_button.setFixedSize(60, 30)
        COM_UpdatePort_button.clicked.connect(self.Update_ComboBox)

        #Button to Connect
        self.COM_button = QtWidgets.QPushButton(self.tr(" Connect COM"))
        self.COM_button.setFixedSize(210, 30)
        self.COM_button.clicked.connect(self.Connect_COM)
        self.COM_button.setIcon(QtGui.QIcon('gui_images/disconnected.png'))

        self.portlist_combobox = QtWidgets.QComboBox()
        self.portlist_combobox = QtWidgets.QComboBox()
        self.portlist_combobox.setFixedSize(150,30)

        #Connection Conatiner
        self.connection_container = QtWidgets.QWidget()
        connection_command = QtWidgets.QVBoxLayout(self.connection_container)

        port_container = QtWidgets.QWidget()
        port_command = QtWidgets.QHBoxLayout(port_container)

        port_command.addWidget(self.portlist_combobox)
        port_command.addWidget(COM_UpdatePort_button)

        port_container.setStyleSheet("border: 1px solid black;")
        self.connection_container.setStyleSheet("border: 1px solid black;")

        connection_command.addStretch()
        connection_command.addWidget(port_container)
        connection_command.setAlignment(port_container,QtCore.Qt.AlignHCenter)
        connection_command.addStretch()
        connection_command.addWidget(self.COM_button)
        connection_command.setAlignment(self.COM_button,QtCore.Qt.AlignHCenter)
        connection_command.addStretch()

    def remoteUI(self):

        self.textbox_dispalydata = QtWidgets.QLineEdit(self, placeholderText="Display")
        self.textbox_dispalydata.setFixedSize(100,30)

        self.remotedevice_combobox = QtWidgets.QComboBox()
        self.remotedevice_combobox = QtWidgets.QComboBox()
        self.remotedevice_combobox.setFixedSize(150,30)
        self.remotedevice_combobox.addItem("Remote 1")
        self.remotedevice_combobox.addItem("Remote 2")

        RemoteLED_button = QtWidgets.QPushButton(self.tr("Toggle Remote LED"))
        RemoteLED_button.setFixedSize(200,30)
        RemoteLED_button.clicked.connect(self.ToggleRemoteLED)

        RemotDisp_button = QtWidgets.QPushButton(self.tr("Remote Dispaly"))
        RemotDisp_button.setFixedSize(120,30)
        RemotDisp_button.clicked.connect(self.UpdateRemoteDisp)


        #Connection Conatiner
        self.remote_container = QtWidgets.QWidget()
        remoteselect_command = QtWidgets.QVBoxLayout(self.remote_container)

        display_container = QtWidgets.QWidget()
        display_command = QtWidgets.QHBoxLayout(display_container)

        display_command.addWidget(self.textbox_dispalydata)
        display_command.addWidget(RemotDisp_button)

        #Remote Select Combobox
        remoteselect_command.addStretch()
        remoteselect_command.addWidget(self.remotedevice_combobox)
        remoteselect_command.setAlignment(self.remotedevice_combobox,QtCore.Qt.AlignHCenter)
        remoteselect_command.addStretch()

        #Remote LED Toggle
        remoteselect_command.addStretch()
        remoteselect_command.addWidget(RemoteLED_button)
        remoteselect_command.setAlignment(RemoteLED_button,QtCore.Qt.AlignHCenter)
        remoteselect_command.addStretch()

        #Remote Select Dispaly Update
        remoteselect_command.addWidget(display_container)
        remoteselect_command.setAlignment(display_container,QtCore.Qt.AlignHCenter)
        remoteselect_command.addStretch()

        self.remote_container.setStyleSheet("border: 1px solid black;")



    def buttonUI(self):
        
        #Request GPS Button
        GPS_button = QtWidgets.QPushButton(self.tr("RequestGPS"))
        GPS_button.setFixedSize(120, 30)
        GPS_button.clicked.connect(self.RequestGPS)

        #Send Custom Command Button
        CustomCommand_button = QtWidgets.QPushButton(self.tr("Send"))
        CustomCommand_button.setFixedSize(60, 30)
        CustomCommand_button.clicked.connect(self.SendCustomCommand)
        

        self.textbox_customsend = QtWidgets.QLineEdit(self, placeholderText="Custom Command")
        self.textbox_customsend.setFixedSize(150,30)


        self.textbox_response = QtWidgets.QLineEdit(self, placeholderText="Response")
        self.textbox_response.setFixedSize(200,30)
        self.textbox_response.setReadOnly(True)
        
        #Map View
        self.view = QtWebEngineWidgets.QWebEngineView()
        self.view.setContentsMargins(20, 50, 50, 50)

        #Main Widget
        central_widget = QtWidgets.QWidget()
        self.setCentralWidget(central_widget)


        lay = QtWidgets.QHBoxLayout(central_widget)

        send_container = QtWidgets.QWidget()
        custom_command = QtWidgets.QHBoxLayout(send_container)
        custom_command.addWidget(self.textbox_customsend)
        custom_command.addWidget(CustomCommand_button)
        
        customdata_container = QtWidgets.QWidget()
        data_recieve = QtWidgets.QVBoxLayout(customdata_container)
        data_recieve.addWidget(send_container)
        data_recieve.addWidget(self.textbox_response)
        button_container = QtWidgets.QWidget()

        vlay = QtWidgets.QVBoxLayout(button_container)
        vlay.setSpacing(10)
        vlay.addStretch()
        vlay.addWidget(self.connection_container)
 
        vlay.setSpacing(10)
        vlay.addStretch()
        vlay.addWidget(self.remote_container)
        vlay.setSpacing(10)
        vlay.addStretch()
        vlay.addWidget(GPS_button)

        vlay.addWidget(customdata_container)
        vlay.addStretch()
        lay.addWidget(button_container)
        lay.addWidget(self.view, stretch=1)

    #Send GPS request and update GUI window
    def RequestGPS(self):
        self.SerialSendCommand("$r")
        while self.ser.in_waiting == 0:
            pass
        gps_data = self.SerialReceiveData()
        if gps_data == 'No_Lock':
            print("No GPS Lock")
        else:   
            lat = gps_data.split(':')[0]
            lon = gps_data.split(':')[1]
            self.Display_GPS(float(lat), float(lon))

    #Update COM port combobox choices once user presses 'Update' button
    def Update_ComboBox(self):
        self.portlist_combobox.clear()
        ports = list_ports.comports()
        for p in ports:
            self.portlist_combobox.addItem(p.device)

    #Try to connect to serial port
    def Connect_COM(self):
        try:
            self.ser = serial.Serial(self.portlist_combobox.currentText(), 9600, timeout=0.5)  # open serial port
            self.COM_button.setIcon(QtGui.QIcon('gui_images/connected.png'))
        except:
            print("Could not connect to ", self.portlist_combobox.currentText())
            self.COM_button.setIcon(QtGui.QIcon('gui_images/disconnected.png'))

    #Update GPS map on GUI window
    def Display_GPS(self,latitude,longitude):
        m = folium.Map(
            location=[latitude, longitude], tiles="OpenStreetMap", zoom_start=13
        )
        tooltip = "Click me!"
        folium.Marker(
        [latitude, longitude], popup="<i>Current Location</i>", tooltip=tooltip).add_to(m)
        data = io.BytesIO()
        m.save(data, close_file=False)
        self.view.setHtml(data.getvalue().decode())

    #Update 8-Seg-Display on remote processor
    def UpdateRemoteDisp(self):
        if (self.ser.is_open):
            if(self.remotedevice_combobox.currentText() == "Remote 1"):
                command = "$x0d" + self.textbox_dispalydata.text()
                self.SerialSendCommand(command)
            else:
                command = "$x1d" + self.textbox_dispalydata.text()
                self.SerialSendCommand(command)

    #Send custom command typed in tetbox
    def SendCustomCommand(self):
        command = self.textbox_customsend.text()
        if command[-2:] == "/n":
            self.SerialSendCommand(command[:-4])
        else:
            self.SerialSendCommand(command)
        self.textbox_customsend.setText("")
        self.SerialReceiveData(FullResponse=True)

    #Toggle remote LED based on remote processor combobox
    def ToggleRemoteLED(self):
        if (self.ser.is_open):
            if(self.remotedevice_combobox.currentText() == "Remote 1"):
                self.SerialSendCommand("$x0l")
            else:
                self.SerialSendCommand("$x1l")

    #Send data through PC connection
    def SerialSendCommand(self,command,addEOL=True):
        if addEOL:
            command += '\r\n'
        self.ser.write(command.encode("ASCII"))

    #Recieve data from PC connection
    def SerialReceiveData(self,FullResponse=False):
        incoming_data = str(self.ser.readline())
        self.textbox_response.setText(incoming_data[2:-5])
        if FullResponse:
            incoming_data = incoming_data[2:-5]
        else:    
            incoming_data = incoming_data[4:-5]
        return incoming_data


if __name__ == "__main__":
    App = QtWidgets.QApplication(sys.argv)
    window = Window()
    window.show()
    sys.exit(App.exec())