import time
import yaml
from datetime import datetime

from collectors.input_tracker import InputTracker
from collectors.app_tracker import AppTracker
from collectors.power_tracker import PowerTracker
from collectors.media_tracker import MediaTracker
from aggregator.window_15m import Window15m
from writer.mongo_writer import MongoWriter


# --------------------------------------------------
# CONFIGURABLE SAMPLING INTERVALS (OPTIMIZED)
# --------------------------------------------------
APP_SAMPLE_INTERVAL = 5      # seconds (foreground app)
MEDIA_SAMPLE_INTERVAL = 10   # seconds (media detection)


# --------------------------------------------------
# TIME OF DAY BUCKET
# --------------------------------------------------
def get_time_bucket():
    hour = datetime.now().hour
    if 5 <= hour < 11:
        return "morning"
    elif 11 <= hour < 16:
        return "afternoon"
    elif 16 <= hour < 21:
        return "evening"
    else:
        return "night"


# --------------------------------------------------
# LOAD CONFIG
# --------------------------------------------------
cfg = yaml.safe_load(open("config.yaml"))
WINDOW = cfg["interval"]["window_seconds"]   # e.g. 900


# --------------------------------------------------
# INIT COMPONENTS
# --------------------------------------------------
input_tracker = InputTracker()
app_tracker = AppTracker()
power_tracker = PowerTracker()
media_tracker = MediaTracker()
window = Window15m()
writer = MongoWriter(cfg["mongodb"], project_dir=".")

input_tracker.start()


# --------------------------------------------------
# MAIN LOOP
# --------------------------------------------------
while True:
    elapsed = 0
    last_app_sample = 0
    last_media_sample = 0

    while elapsed < WINDOW:
        time.sleep(1)
        elapsed += 1

        # ---------- APP SAMPLING ----------
        if elapsed - last_app_sample >= APP_SAMPLE_INTERVAL:
            app = app_tracker.get_active_app()
            state = power_tracker.get_state()

            window.update_app(app, APP_SAMPLE_INTERVAL)

            if state == "active":
                window.active_sec += APP_SAMPLE_INTERVAL
            elif state == "idle":
                window.idle_sec += APP_SAMPLE_INTERVAL
            elif state == "locked":
                window.locked_sec += APP_SAMPLE_INTERVAL

            last_app_sample = elapsed

        # ---------- MEDIA SAMPLING ----------
        if elapsed - last_media_sample >= MEDIA_SAMPLE_INTERVAL:
            media_tracker.sample(MEDIA_SAMPLE_INTERVAL)
            last_media_sample = elapsed

    # ---------- INPUT SNAPSHOT ----------
    keys, mouse = input_tracker.snapshot()
    window.keys += keys
    window.mouse += mouse

    media_data = media_tracker.snapshot()

    # ---------- WRITE TO MONGODB ----------
    writer.write(
        base={
            "device": cfg["device"]["name"],
            "user": cfg["device"]["user"],
            "os": cfg["device"]["os"],
            "time_bucket": get_time_bucket()
        },
        fields={
            "keys_count": window.keys,
            "mouse_count": window.mouse,
            "active_seconds": window.active_sec,
            "idle_seconds": window.idle_sec,
            "locked_seconds": window.locked_sec,

            "apps": window.app_time,
            "app_switch_count": window.app_switches,
            "max_focus_streak_seconds": window.max_focus_streak,

            **media_data
        }
    )

    # ---------- RESET WINDOW ----------
    window.reset()
    media_tracker.reset()
