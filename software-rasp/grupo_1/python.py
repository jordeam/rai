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
builder.add_from_file("respirador.glade")
class Handler:

	def __init__(self):

		style_provider = Gtk.CssProvider()
		with open("main.css", "rb") as f:
			css = f.read()
			style_provider.load_from_data(css)

		Gtk.StyleContext.add_provider_for_screen(
			Gdk.Screen.get_default(),
			style_provider,
			Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
		)

		self.builder = Gtk.Builder()
		self.builder.add_from_file("respirador.glade")
		self.builder.connect_signals(self)
		self.window = self.builder.get_object("window1")
		self.window.show_all()

	def onDestroy(self, *args):
		Gtk.main_quit()
		sock.close()
	
	def on_calcdados(self, dados):
		self.peso =					self.builder.get_object("peso")
		self.altura =					self.builder.get_object("altura")
		self.idade =					self.builder.get_object("idade")
		self.volume_inspirado =			self.builder.get_object("volume_inspirado")
		self.pressao_de_suporte =			self.builder.get_object("pressao_de_suporte")
		self.tempo_de_inspiracao=			self.builder.get_object("tempo_de_inspiracao")
		self.pressao_controlada=			self.builder.get_object("pressao_controlada")
		self.fio2 =					self.builder.get_object("fio2")
		self.tempo_de_subida=				self.builder.get_object("tempo_de_subida")
		self.peep =					self.builder.get_object("peep")
		self.sensibilidade=				self.builder.get_object("sensibilidade")
		self.frequencia_respiratoria=			self.builder.get_object("frequencia_respiratoria")
		peso = float(builder.get_object("peso").get_text())
		volume_inspirado = 7.49e-6 * peso
		wdg=builder.get_object("adj_volume_inspirado")
		wdg.set_value(round(volume_inspirado*1e6))
	
	
	def on_iniciar_activate(self, wdg):
		wdg = builder.get_object("resp_on_label")
		sock.sendall(b'r:start')
                	
	def on_parar_activate(self, wdg):
		wdg = builder.get_object("resp_on_label")
		sock.sendall(b'r:stop')
                
	def adj_volume_inspirado_value_changed_cb(self, wdg):
		volume_inspirado = wdg.get_value();
		sock.sendall(b'r:air! ' + bytearray(str(volume_inspirado), 'utf-8'))

	def adj_tempo_de_inspiracao_value_changed(self, wdg):
		tempo_de_inspiracao = wdg.get_value();
		sock.sendall(b'r:insp-time! ' + bytearray(str(round(tempo_de_inspiracao * 1000)), 'utf-8'))
		exp_time = (60 / resp_freq) - tempo_de_inspiracao
		sock.sendall(b'r:texpn! ' + bytearray(str(round(exp_time * 1000)), 'utf-8'))
                
	def adj_frequencia_respiratoria_value_changed(self, wdg):
		frequencia_respiratoria = wdg.get_value();
		exp_time = (60 / frequencia_respiratoria) - tempo_de_inspiracao
		sock.sendall(b'r:texpn! ' + bytearray(str(round(exp_time * 1000)), 'utf-8'))

	def adj_fio2_value_changed_cb(self, wdg):
		fio2 =  wdg.get_value()
		sock.sendall(b'r:FIO2! ' + bytearray(str(round(fio2)), 'utf-8'))
		
if __name__ == "__main__":
	main = Handler()
	Gtk.main()
