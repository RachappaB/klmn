class Window15m:
    def __init__(self):
        self.reset()

    def reset(self):
        self.app_time = {}
        self.keys = 0
        self.mouse = 0
        self.active_sec = 0
        self.idle_sec = 0
        self.locked_sec = 0
        self.app_switches = 0
        self.last_app = None
        self.focus_streak = 0
        self.max_focus_streak = 0

    def update_app(self, app, sec):
        self.app_time[app] = self.app_time.get(app, 0) + sec

        if app == self.last_app:
            self.focus_streak += sec
        else:
            self.app_switches += 1
            self.focus_streak = sec
            self.last_app = app

        self.max_focus_streak = max(self.max_focus_streak, self.focus_streak)
