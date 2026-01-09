from pynput import keyboard, mouse
from threading import Lock

class InputTracker:
    def __init__(self):
        self.keys = 0
        self.mouse = 0
        self.lock = Lock()

    def start(self):
        keyboard.Listener(on_press=self._on_key).start()
        mouse.Listener(on_click=self._on_mouse).start()

    def _on_key(self, key):
        with self.lock:
            self.keys += 1

    def _on_mouse(self, x, y, button, pressed):
        if pressed:
            with self.lock:
                self.mouse += 1

    def snapshot(self):
        with self.lock:
            k, m = self.keys, self.mouse
            self.keys = 0
            self.mouse = 0
        return k, m
