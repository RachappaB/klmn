import json
from pymongo import MongoClient
from bson import json_util
from datetime import datetime

# =========================
# MongoDB SRV Configuration
# =========================

MONGODB_URI = "mongodb+srv://rachappa:82hZRaXz5rUesKh@cluster0.sfu8ztc.mongodb.net/?appName=Cluster0"

DATABASE_NAME = "productivity"
COLLECTION_NAME = "activity_15m"

OUTPUT_FILE = f"activity_15m_export_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"

# =========================
# Export Logic
# =========================

def export_to_json():
    client = MongoClient(MONGODB_URI)

    collection = client[DATABASE_NAME][COLLECTION_NAME]

    documents = list(collection.find({}))

    with open(OUTPUT_FILE, "w", encoding="utf-8") as file:
        json.dump(
            documents,
            file,
            default=json_util.default,
            indent=2
        )

    print(f"[SUCCESS] Exported {len(documents)} documents to {OUTPUT_FILE}")

    client.close()


if __name__ == "__main__":
    export_to_json()
