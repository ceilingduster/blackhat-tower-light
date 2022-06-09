# https://github.com/miguelgrinberg/Flask-SocketIO/blob/e024b7ec9db4837196d8a46ad1cb82bc1e15f1f3/example/app.py#L30-L31
import time
from pixellib import TowerLight
from threading import Thread, Lock, currentThread, Event
from flask_socketio import SocketIO, emit
from flask import Flask, render_template, jsonify
from typing import Pattern
import eventlet
eventlet.monkey_patch()


app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

thread_lock = Lock()
towerlight = TowerLight()

# null the ring threads
thread = {}
thread_events = {}
for i in range(0, towerlight.num_rings):
    thread_events[i] = Event()
    thread[i] = None


def do_pattern(e, ring_number, pattern_name):
    t = currentThread()
    while True:
        for sequence in sequence_patterns[pattern_name]:
            delay = sequence['delay'] / 1000        # milliseconds
            lights = sequence['lights']
            if 'clear' in sequence:
                clear = sequence['clear']
            else:
                clear = True

            for light in lights:                
                if e.is_set():
                    print("received stop signal")
                    # clear pixels and stop routine
                    towerlight.clearPixels((ring_number * towerlight.num_pixels), (ring_number * towerlight.num_pixels) + towerlight.num_pixels)
                    return None # quit

                light_index = light['index']
                (r, g, b) = light['color']
                for light_number in light_index:
                    real_light_number = (ring_number * towerlight.num_pixels)
                    towerlight.setPixelColor(
                        light_number+(ring_number * towerlight.num_pixels), towerlight.color(r, g, b))

            towerlight.show()
            socketio.emit('light_telemetry', {'pattern_name': pattern_name, 'ring_number': ring_number,
                          'light_index': light_index, 'color': [r, g, b]})      # send telemetry

            socketio.sleep(delay)

            # just clear the pixels we need too, nothing more
            if clear:
                for light in lights:
                    light_index = light['index']
                    for light_number in light_index:
                        real_light_number = (
                            ring_number * towerlight.num_pixels)
                        towerlight.setPixelColor(
                            light_number+(ring_number * towerlight.num_pixels), towerlight.color(0, 0, 0))


@app.route('/start/<ring_number>/<pattern_name>')
def start(ring_number, pattern_name):
    global thread

    ring_number = int(ring_number)
    pattern_name = pattern_name

    with thread_lock:
        thread_events[ring_number] = Event()
        thread[ring_number] = Thread(
            target=do_pattern, args=(thread_events[ring_number], ring_number, pattern_name, ))
        thread[ring_number].start()

    return jsonify(True)


@app.route('/')
def index():
    return render_template('index.html', async_mode=socketio.async_mode)


@app.route('/stop/<ring_number>')
def stop(ring_number):
    global thread

    ring_number = int(ring_number)

    with thread_lock:
        if isinstance(thread[ring_number], Thread):
            if thread[ring_number].is_alive():  # if the thread is alive
                thread_events[ring_number].set()
                socketio.emit('light_telemetry', {
                            'pattern_name': None, 'ring_number': ring_number})      # send telemetrys

    return jsonify(True)


sequence_patterns = towerlight.load_sequences()

if __name__ == '__main__':
    socketio.run(app)
