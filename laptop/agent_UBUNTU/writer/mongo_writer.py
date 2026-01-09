from pymongo import MongoClient, errors
from datetime import datetime
from writer.local_queue import LocalQueue

class MongoWriter:
    def __init__(self, cfg, project_dir):
        self.cfg = cfg
        self.client = None
        self.col = None
        self.queue = LocalQueue(project_dir)
        self._connect()

    def _connect(self):
        try:
            self.client = MongoClient(
                self.cfg["uri"],
                serverSelectionTimeoutMS=3000
            )
            self.client.admin.command("ping")
            db = self.client[self.cfg["database"]]
            self.col = db[self.cfg["collection"]]
            return True
        except:
            self.client = None
            self.col = None
            return False

    def write(self, base, fields):
        document = {
            "timestamp": datetime.utcnow(),
            **base,
            **fields
        }

        if self.col is None:
            self.queue.enqueue(document)
            return

        try:
            self.col.insert_one(document)
            self._flush_queue()
        except errors.PyMongoError:
            self.queue.enqueue(document)
            self.col = None

    def _flush_queue(self):
        for path in self.queue.list_files():
            try:
                with open(path) as f:
                    doc = json.load(f)
                self.col.insert_one(doc)
                self.queue.remove(path)
            except:
                break
