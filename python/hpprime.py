import pygame



class HPPrime:
    def __init__(self, width=320, height=240):
        # Initialize pygame
        pygame.init()
        
        # Create screen and set up window
        self.width = width
        self.height = height
        self.screen = pygame.display.set_mode((self.width, self.height))
        pygame.display.set_caption("HP Prime Graphics Emulation")
        self.screen.fill(0xFFFFFF)
        pygame.display.flip()
        # Set up a clock for frame rate control
        self.clock = pygame.time.Clock()

    def fill(self, color):
        """Fill the screen with a single color."""
        self.screen.fill(color)

    def pixel(self, x, y, color):
        """Draw a single pixel at (x, y)."""
        self.screen.set_at((x, y), color)
        

    def line(self, x1, y1, x2, y2, color, width=1):
        """Draw a line from (x1, y1) to (x2, y2)."""
        pygame.draw.line(self.screen, color, (x1, y1), (x2, y2), width)

    def rect(self, x, y, w, h, color, width=1):
        """Draw a rectangle with top-left corner (x, y) and dimensions (w, h)."""
        pygame.draw.rect(self.screen, color, (x, y, w, h), width)

    def circle(self, x, y, r, color, width=1):
        """Draw a circle at (x, y) with radius r."""
        pygame.draw.circle(self.screen, color, (x, y), r, width)

    def wait(self):
        """Wait until the user closes the window."""
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            pygame.display.flip()
        pygame.quit()

    def close(self):
        """Close the pygame window."""
        pygame.quit()

