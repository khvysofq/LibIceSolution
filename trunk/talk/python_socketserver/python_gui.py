#coding=gbk
 
background_image_filename = 'sushiplate.png'
mouse_image_filename = 'fugu.png'
 
import pygame
from pygame.locals import *
from sys import exit
 
pygame.init()
 
screen = pygame.display.set_mode((640, 480),pygame.RESIZABLE, 32)
pygame.display.set_caption("Hello, World!")
 
background = pygame.image.load(background_image_filename).convert()
#mouse_cursor = pygame.image.load(mouse_image_filename).convert_alpha()
 
while True:
 
    for event in pygame.event.get():
        if event.type == QUIT:
            exit()
 
    screen.blit(background, (0,0))
    pygame.display.update()