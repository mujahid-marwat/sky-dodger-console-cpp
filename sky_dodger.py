import tkinter as tk


WIDTH = 1000
HEIGHT = 700
GROUND = 0
CEILING = 10
MAX_TURNS = 10

BG = "#091522"
CARD = "#112233"
MINT = "#40d0ba"
TEXT = "#f3f7fa"


class SkyDodger:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Sky Dodger | Flight Training")
        self.root.geometry(f"{WIDTH}x{HEIGHT}")
        self.root.resizable(False, False)
        self.root.configure(bg=BG)

        self.canvas = tk.Canvas(
            root,
            width=WIDTH,
            height=HEIGHT,
            bg=BG,
            highlightthickness=0,
            cursor="arrow",
        )
        self.canvas.pack()
        self.canvas.bind("<Button-1>", self.on_click)
        self.canvas.bind("<Motion>", self.on_motion)
        self.root.bind("<KeyPress>", self.on_key)

        self.reset()
        self.draw()
        self.animate()

    def reset(self) -> None:
        self.altitude = 5
        self.score = 0
        self.turns = 0
        self.total_altitude = 0
        self.screen = "ready"
        self.animation_frame = 0

    def start(self) -> None:
        self.reset()
        self.screen = "flying"
        self.draw()

    def move(self, flap: bool) -> None:
        if self.screen != "flying":
            return

        self.altitude += 2 if flap else -1
        if self.altitude <= GROUND or self.altitude >= CEILING:
            self.screen = "crashed"
        else:
            self.turns += 1
            self.score += 1
            self.total_altitude += self.altitude
            if self.turns >= MAX_TURNS:
                self.screen = "cleared"
        self.draw()

    def on_key(self, event: tk.Event) -> None:
        key = event.keysym.lower()
        if key in ("return", "enter") and self.screen in ("ready", "crashed", "cleared"):
            self.start()
        elif key in ("w", "space"):
            if self.screen == "ready":
                self.start()
            elif self.screen in ("crashed", "cleared"):
                self.start()
            self.move(True)
        elif key in ("n", "down"):
            if self.screen == "ready":
                self.start()
            elif self.screen in ("crashed", "cleared"):
                self.start()
            self.move(False)

    def on_click(self, event: tk.Event) -> None:
        x, y = event.x, event.y
        if 48 <= x <= 535 and 610 <= y <= 674:
            if self.screen in ("ready", "crashed", "cleared"):
                self.start()
            else:
                self.move(True)
        elif 551 <= x <= 952 and 610 <= y <= 674:
            if self.screen == "ready":
                self.start()
                self.move(False)
            elif self.screen == "flying":
                self.move(False)

    def on_motion(self, event: tk.Event) -> None:
        over_button = (
            (48 <= event.x <= 535 or 551 <= event.x <= 952)
            and 610 <= event.y <= 674
        )
        self.canvas.configure(cursor="hand2" if over_button else "arrow")

    def animate(self) -> None:
        self.animation_frame += 1
        self.draw()
        self.root.after(40, self.animate)

    def rounded(self, x1: int, y1: int, x2: int, y2: int,
                radius: int, color: str, outline: str = "") -> None:
        self.canvas.create_rectangle(
            x1 + radius, y1, x2 - radius, y2,
            fill=color, outline=color,
        )
        self.canvas.create_rectangle(
            x1, y1 + radius, x2, y2 - radius,
            fill=color, outline=color,
        )
        self.canvas.create_oval(
            x1, y1, x1 + radius * 2, y1 + radius * 2,
            fill=color, outline=color,
        )
        self.canvas.create_oval(
            x2 - radius * 2, y1, x2, y1 + radius * 2,
            fill=color, outline=color,
        )
        self.canvas.create_oval(
            x1, y2 - radius * 2, x1 + radius * 2, y2,
            fill=color, outline=color,
        )
        self.canvas.create_oval(
            x2 - radius * 2, y2 - radius * 2, x2, y2,
            fill=color, outline=color,
        )
        if outline:
            self.canvas.create_line(
                x1 + radius, y1, x2 - radius, y1, fill=outline,
            )
            self.canvas.create_line(
                x1 + radius, y2, x2 - radius, y2, fill=outline,
            )
            self.canvas.create_line(
                x1, y1 + radius, x1, y2 - radius, fill=outline,
            )
            self.canvas.create_line(
                x2, y1 + radius, x2, y2 - radius, fill=outline,
            )
            for cx, cy, start in (
                (x1 + radius, y1 + radius, 90),
                (x2 - radius, y1 + radius, 0),
                (x1 + radius, y2 - radius, 180),
                (x2 - radius, y2 - radius, 270),
            ):
                self.canvas.create_arc(
                    cx - radius, cy - radius, cx + radius, cy + radius,
                    start=start, extent=90, style="arc", outline=outline,
                )

    def label(self, x: int, y: int, text: str, size: int = 14,
              color: str = TEXT, anchor: str = "w",
              weight: str = "normal") -> None:
        self.canvas.create_text(
            x, y, text=text, fill=color, anchor=anchor,
            font=("Segoe UI", size, weight),
        )

    def cloud(self, x: int, y: int) -> None:
        color = "#d5edf8"
        self.canvas.create_oval(x, y + 10, x + 48, y + 34,
                                fill=color, outline=color)
        self.canvas.create_oval(x + 20, y, x + 59, y + 34,
                                fill=color, outline=color)
        self.canvas.create_oval(x + 42, y + 9, x + 82, y + 34,
                                fill=color, outline=color)
        self.canvas.create_rectangle(x + 14, y + 18, x + 68, y + 33,
                                     fill=color, outline=color)

    def draw_bird(self, x: int, y: int) -> None:
        c = self.canvas
        c.create_oval(x - 31, y - 20, x + 31, y + 20,
                      fill="#ffbe4a", outline="#f5a332", width=2)
        c.create_polygon(x - 14, y + 1, x + 5, y + 4, x - 11, y + 14,
                         fill="#f39e33", outline="#f39e33")
        c.create_polygon(x + 26, y - 5, x + 44, y + 1, x + 26, y + 7,
                         fill="#ff835b", outline="#ff835b")
        c.create_oval(x + 10, y - 12, x + 17, y - 5,
                      fill="#142839", outline="#142839")
        c.create_oval(x + 12, y - 11, x + 14, y - 9,
                      fill="white", outline="white")

    def draw_scene(self) -> None:
        c = self.canvas
        self.rounded(48, 132, 952, 442, 18, "#264257", "#304e63")
        c.create_rectangle(52, 136, 948, 438, fill="#3b899c", outline="")
        c.create_rectangle(52, 136, 948, 270, fill="#255d7c", outline="")
        c.create_rectangle(52, 270, 948, 438, fill="#8bc7d2", outline="")
        drift = (self.animation_frame // 2) % 1030
        for x, y, offset in (
            (110, 191, 0), (476, 231, 0), (855, 174, 0),
            (120, 268, 410), (610, 160, 690),
        ):
            self.cloud(52 + ((x - 52 - drift - offset) % 810), y)

        c.create_polygon(
            52, 364, 172, 286, 260, 353, 389, 270, 526, 360,
            648, 296, 786, 365, 884, 302, 948, 349, 948, 438, 52, 438,
            fill="#5aa0a9", outline="",
        )
        c.create_polygon(
            52, 391, 185, 327, 303, 387, 444, 319, 576, 390,
            730, 333, 859, 393, 948, 344, 948, 438, 52, 438,
            fill="#3d848f", outline="",
        )
        c.create_line(76, 171, 920, 171, fill="#effaf6", dash=(3, 4))
        c.create_line(76, 406, 920, 406, fill="#effaf6", dash=(3, 4))
        self.label(77, 157, "CEILING", 10, "#effaf6")
        self.label(77, 418, "GROUND", 10, "#effaf6")

        bird_y = 406 - self.altitude * 235 // CEILING
        self.draw_bird(479, bird_y)
        self.rounded(899, 181, 925, 397, 10, "#18404f", "#dcf3eb")
        meter = max(0, min(210, self.altitude * 210 // CEILING))
        if meter:
            self.rounded(906, 187 + 210 - meter, 918, 397, 6, "#48dfbc")

        if self.screen == "ready":
            self.rounded(284, 251, 675, 329, 14, "#0d2535", "#396a7a")
            self.label(479, 274, "READY FOR TAKEOFF", 21, TEXT,
                       "center", "bold")
            self.label(479, 305, "Flap to climb. Stay between the sky lines.",
                       13, "#b9d2db", "center")
        elif self.screen == "crashed":
            self.rounded(293, 245, 667, 336, 14, "#37202b", "#a35558")
            self.label(480, 269, "FLIGHT ENDED", 21, "#ffcec3",
                       "center", "bold")
            self.label(480, 304, "Your bird flew outside the safe zone.",
                       13, "#e1b9b8", "center")
        elif self.screen == "cleared":
            self.rounded(293, 245, 667, 336, 14, "#133b39", "#43b293")
            self.label(480, 269, "FLIGHT CLEARED!", 21, "#b6ffe0",
                       "center", "bold")
            self.label(480, 304, "Ten turns in the air. Beautiful flying!",
                       13, "#bce2d5", "center")
        else:
            self.label(914, 157, f"ALTITUDE  {self.altitude} / {CEILING}",
                       10, "#f4faf7", "e")

    def metric(self, x: int, title: str, value: str, detail: str,
               accent: str) -> None:
        self.rounded(x, 465, x + 288, 575, 14, CARD, "#22384b")
        self.canvas.create_rectangle(x + 18, 483, x + 22, 557,
                                     fill=accent, outline=accent)
        self.label(x + 36, 487, title, 10, "#97adbe")
        self.label(x + 36, 519, value, 21, "#f2f7fb", weight="bold")
        self.label(x + 36, 552, detail, 10, "#7e97a9")

    def button(self, x1: int, x2: int, title: str, hint: str,
               primary: bool) -> None:
        x, y = self.root.winfo_pointerxy()
        local_x = x - self.root.winfo_rootx()
        local_y = y - self.root.winfo_rooty()
        hovered = x1 <= local_x <= x2 and 610 <= local_y <= 674
        color = (
            "#60e2cc" if hovered else MINT
        ) if primary else ("#21384c" if hovered else "#182b3d")
        self.rounded(x1, 610, x2, 674, 12, color,
                     color if primary else "#32495b")
        main = "#07252b" if primary else "#e7eff5"
        sub = "#1e5052" if primary else "#96adbe"
        self.label(x1 + 22, 630, title, 16, main, weight="bold")
        self.label(x1 + 22, 656, hint, 10, sub)

    def draw(self) -> None:
        c = self.canvas
        c.delete("all")
        c.create_rectangle(0, 0, WIDTH, HEIGHT, fill=BG, outline="")

        c.create_oval(48, 39, 78, 69, fill="#3dd0ba", outline="")
        self.label(63, 54, "S", 16, "#08232d", "center", "bold")
        self.label(91, 45, "SKY DODGER", 21, TEXT, weight="bold")
        self.label(92, 70, "FLIGHT TRAINING  /  SURVIVE THE SKY",
                   10, "#7d99ad")
        self.rounded(781, 38, 952, 70, 16, "#122536", "#2a4254")
        self.label(866, 54, "GROUP  023-26-0022", 10, "#a8becc", "center")
        self.label(48, 106, "Your next move matters.", 30, "#f3f7fa",
                   weight="bold")

        self.draw_scene()
        self.metric(48, "SCORE", f"{self.score:02d}", "points earned", "#45d3bc")
        self.metric(356, "TURN", f"{self.turns} / {MAX_TURNS}",
                    "safe turns survived", "#71b3fa")
        self.metric(664, "ALTITUDE", f"{self.altitude} / {CEILING}",
                    "keep clear of both edges", "#ffbe4a")

        if self.screen == "ready":
            self.button(48, 535, "START FLIGHT",
                        "Click here or press Enter to begin", True)
            self.rounded(551, 610, 952, 674, 12, CARD, "#22384b")
            self.label(571, 630, "HOW TO PLAY", 16, "#dfeaf2", weight="bold")
            self.label(571, 656, "W / click to flap up     N / click to wait",
                       10, "#92abbe")
        elif self.screen in ("crashed", "cleared"):
            self.button(48, 535, "FLY AGAIN",
                        "Click here or press Enter to restart", True)
            self.rounded(551, 610, 952, 674, 12, CARD, "#22384b")
            average = (
                self.total_altitude / self.turns if self.turns else 0.0
            )
            self.label(571, 642,
                       f"FINAL SCORE  {self.score}     AVERAGE ALTITUDE  {average:.2f}",
                       12, "#b5cbd8")
        else:
            self.button(48, 535, "FLAP UP", "+2 altitude     W / SPACE", True)
            self.button(551, 952, "WAIT", "-1 altitude     N / DOWN", False)


def main() -> None:
    root = tk.Tk()
    SkyDodger(root)
    root.mainloop()


if __name__ == "__main__":
    main()
