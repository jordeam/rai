import gi
import math
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

class GUI:
    def __init__(self):
        self.builder = Gtk.Builder()
        self.builder.add_from_file("main.glade")
        self.builder.connect_signals(self)
        self.window = self.builder.get_object("window1")
        self.window.show_all()
   
    def onDestroy(self, *args):
        Gtk.main_quit()

    def on_Calcular1_clicked(self, Calcular1):
        self.Peso1 =              self.builder.get_object("Peso1")
        self.Altura1 =            self.builder.get_object("Altura1")
        self.vol_respirado =      self.builder.get_object("vol_respirado")
        self.tempo_inspira =      self.builder.get_object("tempo_inspira")
        self.frac_C02 =           self.builder.get_object("frac_C02")
        self.Tempo_exp_forcada =  self.builder.get_object("Tempo_exp_forcada")
        self.Press_max_insp =     self.builder.get_object("Press_max_insp")
        self.press_max_exp =      self.builder.get_object("press_max_exp")
        self.time_exp =           self.builder.get_object("time_exp")

        self.vol_respirado = self.builder.get_object("vol_respirado")
        self.grupo = self.builder.get_object("masculino") # variavel do grupo do radio_button
        self.peso = float(self.Peso1.get_text())
        self.altura = float(self.Altura1.get_text())
        self.imc = str(float(self.peso/(self.altura*self.altura)))
        self.vol_respirado.set_text(self.imc)
        self.tempo_inspira.set_text(self.imc)
        self.frac_C02.set_text(self.imc)
        self.Tempo_exp_forcada.set_text(self.imc)
        self.Press_max_insp.set_text(self.imc)
        self.press_max_exp.set_text(self.imc)
        self.time_exp.set_text(self.imc)

if __name__ == "__main__":
    main = GUI()
    Gtk.main()

