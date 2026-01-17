# Claude Pong

Watch Claude play single-player Pong! A terminal-based Pong game where Claude CLI controls the paddle by analyzing the game screen.

## How It Works

- `pong.c` - A simple Pong game engine written in C that renders ASCII frames to a file
- `play.sh` - Game loop that shows Claude the previous and current frame, asks for a move (UP/DOWN/STAY), and advances the game

Claude sees two consecutive frames and must:
1. Track the ball's movement by comparing frames
2. Predict where the ball will be when it reaches the paddle
3. Move early to intercept

## Requirements

- GCC compiler
- [Claude CLI](https://github.com/anthropics/claude-code) authenticated (`claude` command available)
- Bash

## Running

```bash
./play.sh
```

The script will compile the game if needed and start a new game. Press `Ctrl+C` to exit.

## Game Display

```
  Score: 0  Lives: 3
╔══════════════════════════════════════╗
║                                      ║
█                                      ║
█                   ●                  ║
█                                      ║
█                                      ║
║                                      ║
╚══════════════════════════════════════╝
```

- `█` = Paddle (controlled by Claude)
- `●` = Ball
- `╔═╗║╚╝` = Walls

## License

This is free and unencumbered software released into the public domain. See [UNLICENSE](UNLICENSE) for details.
