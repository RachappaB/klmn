import subprocess

class MediaTracker:
    def __init__(self):
        self.media_seconds = 0
        self.media_source = None
        self.media_title = None

    def sample(self, seconds=1):
        """
        Called once per second.
        Counts media time and captures title ONCE per window.
        """
        try:
            status = subprocess.check_output(
                ["playerctl", "status"],
                stderr=subprocess.DEVNULL
            ).decode().strip()

            if status == "Playing":
                self.media_seconds += seconds

                # detect source only once
                if self.media_source is None:
                    self.media_source = self._detect_source()

                # capture title only once
                if self.media_title is None:
                    self.media_title = self._get_title()

        except subprocess.CalledProcessError:
            # No active player — normal case
            pass
        except FileNotFoundError:
            # playerctl not installed
            pass

    def _detect_source(self):
        try:
            players = subprocess.check_output(
                ["playerctl", "-l"],
                stderr=subprocess.DEVNULL
            ).decode().lower()

            if "chrome" in players or "chromium" in players or "firefox" in players:
                return "browser_video"
            if "vlc" in players:
                return "local_video"
            if "spotify" in players:
                return "music"
            return "other_media"

        except:
            return "unknown"

    def _get_title(self):
        try:
            title = subprocess.check_output(
                ["playerctl", "metadata", "xesam:title"],
                stderr=subprocess.DEVNULL
            ).decode().strip()

            # avoid empty or meaningless titles
            if title and title.lower() not in ["", "unknown"]:
                return title[:200]   # safety limit
        except:
            pass

        return None

    def snapshot(self):
        return {
            "background_media": self.media_seconds > 0,
            "background_media_seconds": self.media_seconds,
            "background_media_source": self.media_source,
            "background_media_title": self.media_title
        }

    def reset(self):
        self.media_seconds = 0
        self.media_source = None
        self.media_title = None
