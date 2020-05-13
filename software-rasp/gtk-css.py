import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gdk


class MyWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Hello World")

        style_provider = Gtk.CssProvider()
        with open("main.css", "rb") as f:
            css = f.read()
            style_provider.load_from_data(css)

        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(),
            style_provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

        self.button = Gtk.Button(label="Click Here")
        self.button.connect("clicked", self.on_button_clicked)
        self.add(self.button)
        self.resp_button = Gtk.Button(label="Liga Respirador")
        self.resp_button.connect("clicked", self.on_resp_button_clicked)
        self.add(self.resp_button)

    def on_button_clicked(self, widget):
        print("Hello World")

    def on_resp_button_clicked(self, widget):
        print("r:start")

win = MyWindow()
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()
