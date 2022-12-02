import json
import time
import paho.mqtt.client as paho
import base64
import ssl


class MQTTLib:
    def __init__(self):
        self.client = paho.Client()
        self.client.on_message = self.on_message
        self.client.on_log = self.on_log
        self.client.on_connect = self.on_connect

        # setup the ssl context
        ssl_context = ssl.create_default_context()

        self.client.tls_set(cert_reqs=ssl.CERT_NONE)
        self.client.tls_insecure_set(True)
        self.client.connect("mosquitto", 8883, 60)
        self.client.subscribe("towerlight/update")

        self.debug = True
        self.clients_alive = {}

    def start(self):
        self.client.loop_forever()


    def on_message(self, client, userdata, message):
        self.cleanup_clients()

        try:
            payload = str(message.payload.decode("utf-8"))
            b64d_payload = base64.b64decode(payload)
            json_payload = json.loads(b64d_payload)
            client = list(json_payload)[0]
            schema_ver = json_payload['sc']

            if schema_ver != "1.0":
                raise Exception('invalid schema')

            #print("[%s] received message: %s" % (client, json_payload))
            if client not in self.clients_alive:
                self.clients_alive[client] = []
                self.clients_alive[client].append({'last_timestamp': time.time(), 'status': json_payload[client]})
            else:
                self.clients_alive[client][0]['last_timestamp'] = time.time()

            print("PING! %s" % client)
        except:
            print("Received invalid message.")
            print(message.payload)


    def cleanup_clients(self):
        for (client_id, client_dict) in list(self.clients_alive.items()):
            last_timestamp = client_dict[0]['last_timestamp']

            if time.time() - last_timestamp >= 30:
                self.clients_alive.pop(client_id)

        print("Clients: %s" % json.dumps(self.clients_alive))

    def on_log(self, client, userdata, level, buf):
        if self.debug:
            print("log: ", buf)

    def on_connect(self, client, userdata, flags, rc):
        pass
