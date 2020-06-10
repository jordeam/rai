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
        self.genero = self.get_active_radio()

        self.peso, self.cal_vol_resp = self.volume()

        print("Peso:",self.peso)
        print("Altura:",self.altura)
        print("Gênero:",self.genero)
        print("Volume Respirador:",self.cal_vol_resp)

        self.vol_respirado.set_text(str(self.cal_vol_resp))

    def get_active_radio(self):
        radio_buttons = self.grupo.get_group()
        # Percore a lista de botões e verifica qual botão está ativo
        for radio in radio_buttons:
            if radio.get_active():
                # Retorna o Rótulo do Radio Button que está ativo
                return radio.get_label()

    def volume(self):
        if self.genero == 'Masculino':
            self.peso = 50 + 2.3*(((self.altura*100)*0.394)-60)
            self.cal_vol_resp = 6*self.peso
            round(self.cal_vol_resp,1)

        if self.genero == 'Feminino':
            self.peso = 45.5 + 2.3*(((self.altura*100)*0.394)-60) 
            self.cal_vol_resp = 6*self.peso
            round(self.cal_vol_resp,2)

        return self.peso, self.cal_vol_resp


if __name__ == "__main__":
    main = GUI()
    Gtk.main()

