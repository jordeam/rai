import gi
import math
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk,Gdk

class GUI:

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

	def on_Iniciar(self, iniciar):
		self.Peso1 = self.builder.get_object("peso")
		self.Altura1 =            self.builder.get_object("altura")
		self.vol_respirado =      self.builder.get_object("vol_respirado")
		self.tempo_inspira =      self.builder.get_object("tempo_inspira")
		self.frac_C02 =           self.builder.get_object("frac_C02")
		self.Tempo_exp_forcada =  self.builder.get_object("tempo_exp_forcada")
		self.Press_max_insp =     self.builder.get_object("press_max_insp")
		self.press_max_exp =      self.builder.get_object("press_max_exp")
		self.time_exp =           self.builder.get_object("time_exp")
		
		self.vol_respirado.set_value(2)
		self.vol_respirado.update()


if __name__ == "__main__":
	main = GUI()
	Gtk.main()

