from pynput import keyboard, mouse

class InputTracker:
    def __init__(self):
        self.key_count = 0
        self.mouse_count = 0

    def start(self):
        keyboard.Listener(on_press=self._on_key).start()
        mouse.Listener(on_move=self._on_mouse).start()

    def _on_key(self, key):
        self.key_count += 1

    def _on_mouse(self, x, y):
        self.mouse_count += 1

    def snapshot(self):
        keys = self.key_count
        mouse = self.mouse_count
        self.key_count = 0
        self.mouse_count = 0
        return keys, mouse
