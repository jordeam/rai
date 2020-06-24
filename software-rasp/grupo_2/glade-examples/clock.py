#!/usr/libexec/

import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib

# Thread example
from threading import Thread

import time
from datetime import datetime

class Clock(Thread):
    def __init__(self, label, button):
        """ Clock constructor """
        super(Clock, self).__init__()
        self.label = label
        self.button = button

        # Dummy indicators
        self.stopped = False
        self.alive = True

    def restart(self):
        self.stopped = False

    def stop(self):
        self.stopped = True

    def quit(self):
        self.alive = False

    def __update_clock(self):
        """ Private Method: update widgets """
        now = datetime.now() # current date and time
        if not self.stopped:
            self.label.set_text(now.strftime("%H:%M:%S"))
            self.button.set_label("Stop")
        else:
            self.button.set_label("Continue")

    def run(self):
        while self.alive:
            GLib.idle_add(self.__update_clock)
            # 100ms to prove that update with
            # Threads is able with GLib.
            time.sleep(0.1)

class Handler:
    def __init__(self, label, button):
        self.clock = Clock(label, button)
        self.clock.start()

    def onDestroy(self, *args):
        self.clock.quit()
        Gtk.main_quit()

    def onButtonToggled(self, button):
        if self.clock.stopped:
            self.clock.restart()
        else:
            self.clock.stop()

builder = Gtk.Builder()
builder.add_from_file("clock.glade")

window = builder.get_object("window")
label = builder.get_object("label")
button = builder.get_object("stop")

handler = Handler(label, button)
builder.connect_signals(handler)

window.show_all()

Gtk.main()
