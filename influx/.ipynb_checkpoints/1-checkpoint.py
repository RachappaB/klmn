from influxdb_client import InfluxDBClient
import pandas as pd
import matplotlib.pyplot as plt

url = "https://us-east-1-1.aws.cloud2.influxdata.com"
token = "rcz8tTVL3gNffVUxdTyc_UyCG2ydndmtwKaSlYphEyUPrKXus-qYxhEOaxcU1eMul5g3MAHd_nDhnhhOikG0vg=="

org = "861c8c194327e7ec"
bucket = "s"

client = InfluxDBClient(url=url, token=token, org=org)

query = f'''
from(bucket:"{bucket}")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "iot_data")
'''

df = client.query_api().query_data_frame(query)

print(df)

