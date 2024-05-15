#!/bin/bash

# Start a new screen session in detached mode
screen -dmS Lyrical
screen -dmS Link

# Send the 'cd' command to the screen session
screen -S Lyrical -X stuff 'cd /home/m1st/Lyrical/\n'
screen -S Link -X stuff 'cd /home/m1st/LavaLink\n'

# Send the 'lyrical' command to the screen session
# screen -S Link -X stuff 'nvm use 20.13.1\n'
# screen -S Link -X stuff 'npm start\n'
screen -S Link -X stuff 'java -jar Lavalink.jar\n'
sleep 5
screen -S Lyrical -X stuff './Lyrical\n'