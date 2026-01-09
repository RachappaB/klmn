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
# SAMPLING INTERVALS (OPTIMIZED)
# --------------------------------------------------
APP_SAMPLE_INTERVAL = 5        # seconds
MEDIA_SAMPLE_INTERVAL = 10     # seconds
WINDOW_SECONDS = 900           # 15 minutes


# --------------------------------------------------
# TIME BUCKET
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
with open("config.yaml", "r") as f:
    cfg = yaml.safe_load(f)


# --------------------------------------------------
# INITIALIZE COMPONENTS
# --------------------------------------------------
input_tracker = InputTracker()
app_tracker = AppTracker()
power_tracker = PowerTracker()
media_tracker = MediaTracker()

window = Window15m()
writer = MongoWriter(cfg["mongodb"], ".")

input_tracker.start()

print("Windows productivity agent started")
print("App + input + media tracking enabled")


# --------------------------------------------------
# MAIN LOOP
# --------------------------------------------------
elapsed = 0
last_app_sample = 0
last_media_sample = 0

while True:
    time.sleep(1)
    elapsed += 1

    # ---------------- APP + POWER ----------------
    if elapsed - last_app_sample >= APP_SAMPLE_INTERVAL:
        app = app_tracker.get_active_app()
        state = power_tracker.get_state()

        window.update_app(app, APP_SAMPLE_INTERVAL)

        if state == "active":
            window.active_sec += APP_SAMPLE_INTERVAL
        else:
            window.idle_sec += APP_SAMPLE_INTERVAL

        last_app_sample = elapsed

    # ---------------- MEDIA ----------------
    if elapsed - last_media_sample >= MEDIA_SAMPLE_INTERVAL:
        media_tracker.sample(MEDIA_SAMPLE_INTERVAL)
        last_media_sample = elapsed

    # ---------------- WINDOW COMPLETE ----------------
    if elapsed >= WINDOW_SECONDS:
        # input snapshot
        keys, mouse = input_tracker.snapshot()
        window.keys += keys
        window.mouse += mouse

        media_data = media_tracker.snapshot()

        # write one document
        writer.write(
            base={
                "device": cfg["device"]["name"],
                "user": cfg["device"]["user"],
                "os": "windows",
                "time_bucket": get_time_bucket()
            },
            fields={
                "apps": window.app_time,
                "keys_count": window.keys,
                "mouse_count": window.mouse,

                "active_seconds": window.active_sec,
                "idle_seconds": window.idle_sec,

                "app_switch_count": window.app_switches,
                "max_focus_streak_seconds": window.max_focus_streak,

                **media_data
            }
        )

        # reset window
        window.reset()
        media_tracker.reset()

        elapsed = 0
        last_app_sample = 0
        last_media_sample = 0
