import serial
import threading
import tkinter as tk
from PIL import Image, ImageTk
import pygame

# Set up serial port (make sure the right port is used!)
ser = serial.Serial('/dev/cu.usbmodem1103', 115200, timeout=1)

# Keep track of how many questions have come in
question_count = 0

import time

# Keep checking for messages from the board
def read_from_serial():
    while True:
        if ser.in_waiting:
            line = ser.read(ser.in_waiting).decode(errors='ignore')
            root.after(0, update_question, line)
        time.sleep(0.1)

# What to do when we get new text from serial
def update_question(text):
    global question_count
    if question_count == 0:
        question_label.config(text="")
        door_thread = threading.Thread(target=play_door_sound, daemon=True)
        door_thread.start()
    question_count += 1
    question_label.config(text=text)

# Making the main window and setting it to fullscreen
root = tk.Tk()
root.title("Serial Quiz")
root.attributes('-fullscreen', True)
root.configure(bg="#fef3c7")  # background color of the window
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

# Load the background image
bg_image = Image.open("indiana.jpg")
bg_photo = ImageTk.PhotoImage(bg_image.resize((screen_width, screen_height)))
bg_label = tk.Label(root, image=bg_photo)
bg_label.place(x=0, y=0, relwidth=1, relheight=1)


question_label = tk.Label(
    root,
    text="",
    font=("Papyrus", 24, "bold"),
    wraplength=screen_width - 800,  # Make the text wrap nicely in the middle
    justify="center",
    bg="#d2b48c",  # background color for the label
    fg="#000000",  # text color
    bd=10,  # Nice thick border so it looks better
    relief="ridge",
    padx=10,  # Padding inside the label (left/right and top/bottom)
    pady=10,   # Padding inside the label (left/right and top/bottom)
    width=50,
)
question_label.pack(pady=50, expand=True)

question_label.config(text="Welcome to the LED challenge!\n\n"
                           "In this task you must click the button and the target LED is shown\n"
                           "Your task is to stop the moving LED on the target\n"
                           "There will be 3 levels of increasing difficulty\n"
                           "Good luck!\n\n"
                           "Press the button to start the game\n")

thread = threading.Thread(target=read_from_serial, daemon=True)
thread.start()

pygame.mixer.init()

def play_intro_sound():
    pygame.mixer.music.load("indiana.mp3")
    pygame.mixer.music.set_volume(1.0)
    pygame.mixer.music.play(-1)

def play_door_sound():
    # Play the door sound and lower background music a bit
    pygame.mixer.music.set_volume(0.1)
    sound = pygame.mixer.Sound("door_open.mp3")
    sound.play()

# Start the background music
sound_thread = threading.Thread(target=play_intro_sound, daemon=True)
sound_thread.start()

root.mainloop()