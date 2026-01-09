import json
from pathlib import Path
from datetime import datetime

class LocalQueue:
    def __init__(self, base_dir):
        self.queue_dir = Path(base_dir) / "offline_queue"
        self.queue_dir.mkdir(exist_ok=True)

    def enqueue(self, document):
        fname = datetime.utcnow().strftime("%Y%m%d_%H%M%S_%f.json")
        path = self.queue_dir / fname
        with open(path, "w") as f:
            json.dump(document, f)

    def list_files(self):
        return sorted(self.queue_dir.glob("*.json"))

    def remove(self, path):
        path.unlink()
