from pixellib import TowerLight

towerlight = TowerLight()
towerlight.setBrightness(255)
towerlight.setPixelColor(1, towerlight.color(255, 0, 0))
towerlight.show()