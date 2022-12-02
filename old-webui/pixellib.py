from rpi_ws281x import PixelStrip, Color
import os
import json


class TowerLight:
    def __init__(self, sequence_folder_path='./sequences'):
        # global constants
        self.num_pixels = 12
        self.num_rings = 3
        self.total_pixels = self.num_pixels * self.num_rings
        self.sequence_folder = sequence_folder_path

        # constants for the neopixel strip
        self._led_pin = 18
        self._freq_hz = 800000
        self._dma = 10
        self._brightness = 255
        self._invert = False
        self._channel = 0

        # load all the sequences
        self.sequences = self.load_sequences()
        # GPIO pin connected to the pixels (18 uses PWM!).
        LED_PIN = 18
        LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
        # DMA channel to use for generating signal (try 10)
        LED_DMA = 10
        LED_BRIGHTNESS = 128  # Set to 0 for darkest and 255 for brightest
        # True to invert the signal (when using NPN transistor level shift)
        LED_INVERT = False
        LED_CHANNEL = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53
        NUMPIXELS = 12
        NUMRINGS = 3
        TOTALPIXELS = NUMPIXELS * NUMRINGS

        self.strip = PixelStrip(num=TOTALPIXELS,
                                pin=LED_PIN,
                                freq_hz=LED_FREQ_HZ,
                                dma=LED_DMA,
                                invert=LED_INVERT,
                                brightness=LED_BRIGHTNESS,
                                channel=LED_CHANNEL,
                                strip_type=0x18081000)

        self.strip.begin()

    @staticmethod
    def color(r, g, b):
        return Color(r, g, b)

    def load_sequences(self):
        sequence_files = [x for x in os.listdir(
            self.sequence_folder) if x.endswith('.json')]
        sequence_data = {}
        for sequence_file in sequence_files:
            sequence_file_path = os.path.join(
                self.sequence_folder, sequence_file)
            with open(sequence_file_path, 'r') as f:
                data = json.load(f)
                for sequence_name in data.keys():
                    sequence_data[sequence_name] = data[sequence_name]
        return sequence_data

    def setPixelColor(self, pixel_number, color):
        self.strip.setPixelColor(pixel_number, color)

    def show(self):
        self.strip.show()

    def setBrightness(self, brightness):
        self.strip.setBrightness(brightness)

    def clearPixels(self, start, end):
        for pixel_number in range(start, end):
            self.strip.setPixelColor(pixel_number, self.color(0, 0, 0))
            self.strip.show()

    def clear(self):
        for i in range(self.strip.numPixels()):
            self.strip.setPixelColor(i, Color(0, 0, 0))
            self.strip.show()
