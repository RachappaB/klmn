import time
import ctypes

class PowerTracker:
    def __init__(self):
        self.last_input = time.time()

    def get_state(self):
        class LASTINPUTINFO(ctypes.Structure):
            _fields_ = [("cbSize", ctypes.c_uint),
                        ("dwTime", ctypes.c_uint)]

        li = LASTINPUTINFO()
        li.cbSize = ctypes.sizeof(LASTINPUTINFO)
        ctypes.windll.user32.GetLastInputInfo(ctypes.byref(li))

        idle_time = (ctypes.windll.kernel32.GetTickCount() - li.dwTime) / 1000
        return "idle" if idle_time > 60 else "active"
