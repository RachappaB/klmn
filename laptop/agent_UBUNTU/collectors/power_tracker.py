import subprocess
import os

class PowerTracker:
    def get_state(self):
        try:
            session = os.environ.get("XDG_SESSION_ID")
            if not session:
                return "unknown"

            idle = subprocess.check_output(
                ["loginctl", "show-session", session, "-p", "IdleHint"],
                stderr=subprocess.DEVNULL
            ).decode().lower()

            locked = subprocess.check_output(
                ["loginctl", "show-session", session, "-p", "LockedHint"],
                stderr=subprocess.DEVNULL
            ).decode().lower()

            if "yes" in locked:
                return "locked"
            if "yes" in idle:
                return "idle"
            return "active"
        except:
            return "unknown"
