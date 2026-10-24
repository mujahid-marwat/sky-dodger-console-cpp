# Sky Dodger

A small altitude-survival game. The primary playable version is a graphical
Python app built with Tkinter.

## Run the game

Requires Python 3 with Tkinter available (Tkinter is included with most standard
Python installations).

From this directory, run:

```bash
python sky_dodger.py
```

On Windows, you can also double-click `run_game.bat`. It uses the system Python
and does not require the project's local virtual environment.

## Controls

- **W** or **Space**: flap upward
- **N** or **Down Arrow**: wait and descend
- **Enter**: start or restart
- You can also use the on-screen buttons.

Stay between the ground and ceiling for ten turns to clear a flight.

## Other source files

- `pf_assignment_for_github.cpp` is the supplied C++ console assignment source.
- `023-26-0022.cpp` is a separate Windows-native C++ GUI implementation.

The Python GUI is the recommended version to run. The C++ files are source
only; no compiled executables are included.
