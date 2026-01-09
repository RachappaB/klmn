import subprocess

class AppTracker:
    def get_active_app(self):
        try:
            name = subprocess.check_output(
                ["xdotool", "getwindowfocus", "getwindowname"],
                stderr=subprocess.DEVNULL
            ).decode(errors="ignore").strip()
            return name[:80]
        except:
            return "unknown"
