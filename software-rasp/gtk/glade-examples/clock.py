#!/usr/libexec/

import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib

# Thread example
from threading import Thread

import time
from datetime import datetime

# Dummy lock
stop = False
run = True

class Clock(Thread):
    def __init__(self, label, button):
        """ Clock constructor """
        super(Clock, self).__init__()
        self.label = label
        self.button = button

    def __update_clock(self):
        """ Private Method: update widgets """
        now = datetime.now() # current date and time
        if not stop:
            self.label.set_text(now.strftime("%H:%M:%S"))
            self.button.set_label("Stop")
        else:
            self.button.set_label("Continue")

    def run(self):
        while run:
            GLib.idle_add(self.__update_clock)
            time.sleep(0.1)

class Handler:
    def onDestroy(self, *args):
        global run

        run = False
        Gtk.main_quit()

    def onButtonClick(self, button):
        global stop

        # inline true or false (similar C syntax)
        stop = False if stop else True

builder = Gtk.Builder()
builder.add_from_file("clock.glade")
builder.connect_signals(Handler())

window = builder.get_object("window")
label = builder.get_object("label")
button = builder.get_object("stop")

clock = Clock(label, button)
clock.start()

window.show_all()

Gtk.main()
