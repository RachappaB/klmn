class Window15m:
    def __init__(self):
        self.reset()

    def reset(self):
        self.keys = 0
        self.mouse = 0

        self.active_sec = 0
        self.idle_sec = 0
        self.locked_sec = 0

        self.app_time = {}
        self.app_switches = 0

        self.last_app = None
        self.current_streak = 0
        self.max_focus_streak = 0

    def update_app(self, app, seconds=1):
        self.app_time[app] = self.app_time.get(app, 0) + seconds

        if self.last_app is not None and self.last_app != app:
            self.app_switches += 1
            self.current_streak = 0

        self.current_streak += seconds
        if self.current_streak > self.max_focus_streak:
            self.max_focus_streak = self.current_streak

        self.last_app = app
