#!/bin/bash

# Start a new screen session in detached mode
screen -dmS Lyrical
screen -dmS NodeLink

# Send the 'cd' command to the screen session
screen -S Lyrical -X stuff 'cd ~/Lyrical/out\n'
screen -S NodeLink -X stuff 'cd ~/NodeLink\n'

# Send the 'lyrical' command to the screen session
screen -S NodeLink -X stuff 'nvm use 20.13.1\n'
screen -S NodeLink -X stuff 'npm start\n'
sleep 1
screen -S Lyrical -X stuff './Lyrical\n'