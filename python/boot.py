from hpprime import HPPrime
#import pygame


# Initialize HPPrimeGraphics with default screen size
hpprime = HPPrime()

# Fill screen with white
#hpprime.fill((255, 255, 255))

# Draw some shapes
#hpprime.pixel(50, 50, (255, 0, 0))  # Red pixel at (50, 50)
#hpprime.line(10, 10, 200, 200, (0, 0, 255), 2)  # Blue line from (10, 10) to (200, 200)
#graphics.rect(100, 50, 80, 60, (0, 255, 0), 2)  # Green rectangle at (100, 50) with width 80 and height 60
#hpprime.circle(160, 120, 40, (255, 0, 0), 2)  # Red circle with center (160, 120) and radius 40

# Update the display to show the drawings
#graphics.update()

for y in range(240):
  for x in range(320):
    hpprime.pixel(x, y, (255, 0, 0))

# Wait for the user to close the window
hpprime.wait()

      

# Optionally, close pygame
hpprime.close()
