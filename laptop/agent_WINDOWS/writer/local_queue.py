import json
from pathlib import Path
from datetime import datetime

class LocalQueue:
    def __init__(self, base_dir):
        self.dir = Path(base_dir) / "offline_queue"
        self.dir.mkdir(exist_ok=True)

    def enqueue(self, doc):
        path = self.dir / f"{datetime.utcnow().isoformat()}.json"
        with open(path, "w") as f:
            json.dump(doc, f)

    def files(self):
        return sorted(self.dir.glob("*.json"))

    def remove(self, path):
        path.unlink()
