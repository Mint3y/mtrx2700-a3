import tkinter as tk

class ShapeGame:
    def __init__(self, root):
        self.root = root
        self.root.title("Shape Monster Game")
        self.screen_width = root.winfo_screenwidth()
        self.screen_height = root.winfo_screenheight()
        self.canvas = tk.Canvas(root, width=self.screen_width, height=self.screen_height, bg="lightblue")
        self.canvas.pack()

        center_x = self.screen_width // 2
        center_y = self.screen_height // 2 - 100
        self.monsters = {
            'triangle': self.canvas.create_polygon(center_x - 300, center_y, center_x - 250, center_y + 100, center_x - 350, center_y + 100, fill="red"),
            'square': self.canvas.create_rectangle(center_x - 100, center_y, center_x, center_y + 100, fill="pink"),
            'circle': self.canvas.create_oval(center_x + 100, center_y, center_x + 200, center_y + 100, fill="green"),
            'rectangle': self.canvas.create_rectangle(center_x + 250, center_y + 10, center_x + 400, center_y + 70, fill="orange"),
        }

        self.drag_data = {"x": 0, "y": 0}
        self.shapes = ['triangle', 'square', 'circle', 'rectangle']
        self.current_shape_index = 0

        self.shape = self.create_shape('triangle')

        self.canvas.tag_bind("draggable", "<ButtonPress-1>", self.start_drag)
        self.canvas.tag_bind("draggable", "<ButtonRelease-1>", self.stop_drag)
        self.canvas.tag_bind("draggable", "<B1-Motion>", self.do_drag)

    def create_shape(self, shape_type):
        spawn_x = self.screen_width // 2
        spawn_y = self.screen_height // 2 + 150
        if shape_type == 'triangle':
            return self.canvas.create_polygon(spawn_x, spawn_y, spawn_x + 50, spawn_y + 100, spawn_x - 50, spawn_y + 100, fill="lightblue", outline="black", width=2, tags="draggable")
        elif shape_type == 'square':
            return self.canvas.create_rectangle(spawn_x - 50, spawn_y, spawn_x + 50, spawn_y + 100, fill="lightblue", outline="black", width=2, tags="draggable")
        elif shape_type == 'circle':
            return self.canvas.create_oval(spawn_x - 50, spawn_y, spawn_x + 50, spawn_y + 100, fill="lightblue", outline="black", width=2, tags="draggable")
        elif shape_type == 'rectangle':
            return self.canvas.create_rectangle(spawn_x - 50, spawn_y - 50, spawn_x + 50, spawn_y, fill="lightblue", outline="black", width=2, tags="draggable")

    def start_drag(self, event):
        self.drag_data["x"] = event.x
        self.drag_data["y"] = event.y

    def stop_drag(self, event):
        self.canvas.delete("all_text")
        x, y = event.x, event.y
        current_shape = self.shapes[self.current_shape_index]
        coords = self.canvas.coords(self.monsters[current_shape])
        min_x = min(coords[::2])
        max_x = max(coords[::2])
        min_y = min(coords[1::2])
        max_y = max(coords[1::2])

        if min_x <= x <= max_x and min_y <= y <= max_y:
            self.canvas.create_text(self.screen_width // 2, self.screen_height // 2, text="Correct!", font=("Arial", 24), fill="green", tags="all_text")
        else:
            self.canvas.create_text(self.screen_width // 2, self.screen_height // 2, text="Try again!", font=("Arial", 24), fill="red", tags="all_text")

        self.canvas.delete(self.shape)
        self.current_shape_index = (self.current_shape_index + 1) % len(self.shapes)
        self.shape = self.create_shape(self.shapes[self.current_shape_index])

    def do_drag(self, event):
        dx = event.x - self.drag_data["x"]
        dy = event.y - self.drag_data["y"]
        self.canvas.move("draggable", dx, dy)
        self.drag_data["x"] = event.x
        self.drag_data["y"] = event.y

root = tk.Tk()
game = ShapeGame(root)
root.mainloop()