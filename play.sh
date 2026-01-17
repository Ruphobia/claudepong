#!/bin/bash

# Claude Pong - Watch Claude play single-player pong

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Log file
LOG="game.log"
echo "=== Game started $(date) ===" > "$LOG"
echo "USER: $(whoami)" >> "$LOG"
echo "HOME: $HOME" >> "$LOG"
echo "Which claude: $(which claude)" >> "$LOG"
echo "Credentials exist: $(test -f ~/.claude/.credentials.json && echo YES || echo NO)" >> "$LOG"

# ANSI escape sequences
ESC="\033"
CLEAR="${ESC}[2J"
CURSOR_HOME="${ESC}[H"
HIDE_CURSOR="${ESC}[?25l"
SHOW_CURSOR="${ESC}[?25h"

# Cleanup on exit
cleanup() {
    printf "${SHOW_CURSOR}"
    echo "=== Game ended $(date) ===" >> "$LOG"
    exit 0
}
trap cleanup EXIT INT TERM

# Compile if needed
if [ ! -f "./pong" ] || [ "./pong.c" -nt "./pong" ]; then
    echo "Compiling pong..."
    gcc -o pong pong.c
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Initialize game
./pong init
cp game.txt prev_frame.txt

# Setup terminal
printf "${HIDE_CURSOR}"
printf "${CLEAR}"

# Game loop
TURN=0
while true; do
    TURN=$((TURN + 1))

    # Move cursor to home and draw frame
    printf "${CURSOR_HOME}"
    echo "  Turn: $TURN"
    cat game.txt
    echo ""
    echo "  Thinking..."

    # Check if game is over
    STATUS=$(./pong status)
    if [ "$STATUS" = "gameover" ]; then
        printf "${CURSOR_HOME}"
        echo "  Turn: $TURN"
        cat game.txt
        echo ""
        SCORE=$(grep "Score:" game.txt | awk '{print $2}')
        echo "  Final Score: $SCORE"
        echo "                    "
        break
    fi

    # Ask Claude for a move
    PROMPT="PONG - You are the paddle (█) on the left. Block the ball (●)!

Compare these two frames to see which way the ball is moving:

PREVIOUS FRAME:
$(cat prev_frame.txt)

CURRENT FRAME:
$(cat game.txt)

RULES:
- Ball moves diagonally each turn and bounces off walls
- You move 1 row per turn
- Predict where ball will be when it reaches the left wall
- Move early to intercept!

Reply ONLY: UP, DOWN, or STAY"

    # Log the prompt
    echo "--- Turn $TURN ---" >> "$LOG"
    echo "$PROMPT" >> "$LOG"

    # Get Claude's move
    RAW_RESPONSE=$(claude -p "$PROMPT" 2>&1)
    MOVE=$(echo "$RAW_RESPONSE" | grep -oE "(UP|DOWN|STAY)" | head -1)

    # Log raw response
    echo "Raw response: $RAW_RESPONSE" >> "$LOG"
    echo "Extracted move: $MOVE" >> "$LOG"

    # Default to STAY if we didn't get a valid move
    if [ -z "$MOVE" ]; then
        MOVE="STAY"
        echo "Defaulted to STAY" >> "$LOG"
    fi

    # Redraw with move info
    printf "${CURSOR_HOME}"
    echo "  Turn: $TURN"
    cat game.txt
    echo ""
    echo "  Claude: $MOVE    "

    # Save current frame as previous before move
    cp game.txt prev_frame.txt

    # Apply move and advance game
    ./pong move "$MOVE"

    # Small delay so you can watch
    sleep 0.3
done
