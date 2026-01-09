import json
from pymongo import MongoClient
from datetime import datetime
from writer.local_queue import LocalQueue

class MongoWriter:
    def __init__(self, cfg, base_dir):
        self.queue = LocalQueue(base_dir)
        try:
            self.client = MongoClient(cfg["uri"], serverSelectionTimeoutMS=3000)
            self.client.admin.command("ping")
            self.col = self.client[cfg["database"]][cfg["collection"]]
        except:
            self.col = None

    def write(self, base, fields):
        doc = {"timestamp": datetime.utcnow(), **base, **fields}

        if not self.col:
            self.queue.enqueue(doc)
            return

        try:
            self.col.insert_one(doc)
            for f in self.queue.files():
                with open(f) as fh:
                    self.col.insert_one(json.load(fh))
                self.queue.remove(f)
        except:
            self.queue.enqueue(doc)
            self.col = None
