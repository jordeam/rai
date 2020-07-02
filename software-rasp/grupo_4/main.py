import gi
import math
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk,Gdk
import socket
import sys
import time
import subprocess

# Calls fake-arm
# subprocess.call(['../../fake-arm/fake-arm'])

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect the socket to the port where the server is listening
server_address = ('localhost', 5005)
print (sys.stderr, 'Conectando com %s porta %s' % server_address)

try:
        sock.connect(server_address)
except socket.error:
        print("Process `fake-arm' must be running")
#        exit(1)

builder = Gtk.Builder()
builder.add_from_file("main.glade")

# Ventilator Parameters
# Inspiration Volume
FIO2 = 20
insp_vol = 400
insp_time = 0.8
resp_freq = 35
ge = 30
height = 1.70
weight = 60

class Handler:

        def onDestroy(self, *args):
                Gtk.main_quit()
                sock.close()

        def on_eval_data_clicked(self, button):
                print("Cálculo baseado nos dados do paciente")
                age = int(builder.get_object("age").get_text())
                print("  idade = ", age)
                height = float(builder.get_object("height").get_text())
                print("  altura = ", height)
                weight = float(builder.get_object("weight").get_text())
                print("  peso = ", weight)
                if builder.get_object("masc").get_active():
                        print ("  Masculino")
                else:
                        print("  Feminino")
                if builder.get_object("decease_press").get_active():
                        print ("  pressão alta")
                if builder.get_object("decease_diab_1").get_active():
                        print ("  diabetes tipo 1")
                if builder.get_object("decease_diab_2").get_active():
                        print ("  diabetes tipo 2")
                insp_vol = 7.49e-6 * weight
                wdg = builder.get_object("adj_vol")
                wdg.set_value(round(insp_vol*1e6))
                
        def onDiab1Toggled(self, button):
                if button.get_active():
                        btn = builder.get_object("decease_diab_2")
                        btn.set_active(False)
                        
        def onDiab2Toggled(self, button):
                if button.get_active():
                        btn = builder.get_object("decease_diab_1")
                        btn.set_active(False)

        def on_resp_on_state_set(self, wdg, v):
                wdg = builder.get_object("resp_on_label")
                if v:
                        sock.sendall(b'r:start')
                        print("Resp ON");
                        wdg.set_text("Respirador ativo")
                else:
                        print("resp OFF")
                        sock.sendall(b'r:stop')
                        wdg.set_text("Processo pausado")

        def on_adj_vol_value_changed(self, wdg):
                insp_vol = wdg.get_value();
                print("insp_vol = ", insp_vol)
                sock.sendall(b'r:air! ' + bytearray(str(insp_vol), 'utf-8'))

        def on_adj_time_insp_value_changed(self, wdg):
                insp_time = wdg.get_value();
                print("insp_time = ", insp_time)
                sock.sendall(b'r:insp-time! ' + bytearray(str(round(insp_time * 1000)), 'utf-8'))
                exp_time = (60 / resp_freq) - insp_time
                print("exp_time = ", exp_time)
                sock.sendall(b'r:texpn! ' + bytearray(str(round(exp_time * 1000)), 'utf-8'))
                
        def on_adj_freq_value_changed(self, wdg):
                resp_freq = wdg.get_value();
                exp_time = (60 / resp_freq) - insp_time
                print("exp_time = ", exp_time)
                sock.sendall(b'r:texpn! ' + bytearray(str(round(exp_time * 1000)), 'utf-8'))

        def on_adj_fio2_value_changed(self, wdg):
                FIO2 =  wdg.get_value()
                sock.sendall(b'r:FIO2! ' + bytearray(str(round(FIO2)), 'utf-8'))
                
builder.connect_signals(Handler())

window = builder.get_object("window1")
window.show_all()

Gtk.main()
