import gi
import math
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk,Gdk
import socket
import sys
import time


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect the socket to the port where the server is listening
server_address = ('localhost', 5000)
print (sys.stderr, 'Conectando com %s porta %s' % server_address)
sock.connect(server_address)

builder = Gtk.Builder()
builder.add_from_file("main.glade")

class Handler:

        def onDestroy(self, *args):
                Gtk.main_quit()

        def onEvalDataClicked(self, button):
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
                        
        def onDiab1Toggled(self, button):
                if button.get_active():
                        btn = builder.get_object("decease_diab_2")
                        btn.set_active(False)
                        
        def onDiab2Toggled(self, button):
                if button.get_active():
                        btn = builder.get_object("decease_diab_1")
                        btn.set_active(False)

        def onRespActivate(self, wdg, v):
                if v:
                        sock.sendall(b'r:start')
                        print("Resp ON");
                else:
                        print("resp OFF")
                        sock.sendall(b'r:stop')
                        
builder.connect_signals(Handler())

window = builder.get_object("window1")
window.show_all()

Gtk.main()
