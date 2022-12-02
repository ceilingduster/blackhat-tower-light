from base64 import b64encode
import os
import time
import json
from flask import Flask, jsonify, request, Response, render_template
from threading import Thread
from mqttlib import MQTTLib

app = Flask(__name__)
mqtt = MQTTLib()

@app.route('/clients_available')
def client_list():
    return mqtt.clients_alive

@app.route('/')
def main_index():
    return render_template('base.html')

@app.route('/lights_off')
def lights_off():
    args = request.args
    client_id = args.get("c", default=None, type=str)
    if client_id == None:
        return 'You must specify a Client ID!'

    for ring_number in [0, 1, 2]:
        pattern_num = 0
        interval_value = 1000
        red_value = 0
        green_value = 0
        blue_value = 0

        json_command = json.dumps({"to": client_id, "p": pattern_num, "i": interval_value, "r": red_value,
        "g": green_value, "b": blue_value, "n": ring_number})
        mqtt.client.publish("towerlight/command", json_command)

    return Response(jsonify(True), mimetype='application/json')


@app.route('/control')
def publish():
    args = request.args
    pattern_num = args.get("p", default=3, type=int)
    red_value = args.get("r", default=255, type=int)
    green_value = args.get("g", default=255, type=int)
    blue_value = args.get("b", default=255, type=int)
    interval_value = args.get("i", default=1000, type=int)
    ring_number = args.get("n", default=0, type=int)

    client_id = args.get("c", default=None, type=str)
    if client_id == None:
        return 'You must specify a Client ID!'
    else:
        json_command = json.dumps({"to": client_id, "p": pattern_num, "i": interval_value, "r": red_value,
        "g": green_value, "b": blue_value, "n": ring_number})

        mqtt.client.publish("towerlight/command", json_command)

        return Response(jsonify(True), mimetype='application/json')


if __name__ == '__main__':
    thread = Thread(target=mqtt.start)
    thread.daemon = True
    thread.start()
    app.run(host='0.0.0.0', port=80)
