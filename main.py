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
        self.Peso1 = self.builder.get_object("Peso1")
        self.Altura1 = self.builder.get_object("Altura1")
        self.vol_respirado = self.builder.get_object("vol_respirado")
        self.peso = float(self.Peso1.get_text())
        self.altura = float(self.Altura1.get_text())
        self.imc = str(float(self.peso/(self.altura*self.altura)))
        print("Peso",self.peso)
        print("Altura",self.altura)
        self.vol_respirado.set_text(self.imc)

if __name__ == "__main__":
    main = GUI()
    Gtk.main()

