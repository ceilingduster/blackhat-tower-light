import json
import time
import paho.mqtt.client as paho
import base64
import ssl

clients_alive = {}
debug = True

def on_message(client, userdata, message):
    global clients_alive
    cleanup_clients()

    try:
        payload = str(message.payload.decode("utf-8"))
        b64d_payload = base64.b64decode(payload)
        json_payload = json.loads(b64d_payload)
        client = list(json_payload)[0]
        schema_ver = json_payload['sc']

        if schema_ver != "1.0":
            raise Exception('invalid schema')

        #print("[%s] received message: %s" % (client, json_payload))
        if client not in clients_alive:
            clients_alive[client] = []
            clients_alive[client].append({'last_timestamp': time.time()})
        else:
            clients_alive[client][0]['last_timestamp'] = time.time()

        print("PING! %s" % client)
    except:
        print("Received invalid message.")
        print(message.payload)


def cleanup_clients():
    global clients_alive

    for (client_id, client_dict) in list(clients_alive.items()):
        last_timestamp = client_dict[0]['last_timestamp']

        if time.time() - last_timestamp >= 30:
            clients_alive.pop(client_id)


    print("Clients: %s" % json.dumps(clients_alive))

def on_log(client, userdata, level, buf):
    if debug:
        print("log: ", buf)


def on_connect(client, userdata, flags, rc):
    pass


client = paho.Client()
client.on_message = on_message
client.on_log = on_log
client.on_connect = on_connect
ssl_context = ssl.create_default_context()

client.tls_set(cert_reqs=ssl.CERT_NONE)
client.tls_insecure_set(True)
client.connect("localhost", 8443, 60)
client.subscribe("towerlight/update")

client.publish("towerlight/command", json.dumps({"to": "30:C6:F7:25:C4:38", "p": 3,  "i": 1000,  "r": 255,  "g": 0,  "b": 0, "n": 2}))

client.loop_forever()
