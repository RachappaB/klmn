import asyncio
from winrt.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)

class MediaTracker:
    def __init__(self):
        self.media_seconds = 0
        self.media_title = None
        self.media_source = None
        self.manager = None

    async def _get_session(self):
        if self.manager is None:
            self.manager = await MediaManager.request_async()
        return self.manager.get_current_session()

    def sample(self, seconds):
        try:
            session = asyncio.run(self._get_session())
            if not session:
                return

            info = session.get_playback_info()
            if info.playback_status.name != "PLAYING":
                return

            self.media_seconds += seconds
            self.media_source = session.source_app_user_model_id

            if self.media_title is None:
                props = asyncio.run(session.try_get_media_properties_async())
                self.media_title = props.title

        except Exception:
            pass

    def snapshot(self):
        return {
            "background_media": self.media_seconds > 0,
            "background_media_seconds": self.media_seconds,
            "background_media_source": self.media_source,
            "background_media_title": self.media_title
        }

    def reset(self):
        self.media_seconds = 0
        self.media_title = None
        self.media_source = None
