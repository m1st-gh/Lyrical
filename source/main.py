import nextcord 
import nextcord.ext
from nextcord.ext import commands, tasks
import queue
import logging
import datetime
import os
from lib import config
from yt_dlp import YoutubeDL
from cogs import musicCommand as music
from lib.config import config as tokens
import asyncio



FFMPEG_OPTIONS = {'before_options': '-reconnect 1 -reconnect_streamed 1 -reconnect_delay_max 5', 'options': '-vn'}
YDL_OPTIONS = {'format': 'bestaudio', 'noplaylist': 'True'}
Lyrical = commands.Bot()
logging.basicConfig(level=logging.INFO)

token = tokens.get_token()
#guild_id = tokens.get_guild_id()

@Lyrical.event
async def on_ready():
    start = datetime.datetime.now()
    while True:
        current_time = datetime.datetime.now()
        uptime = current_time - start
        presence = f"Uptime: {int(uptime.total_seconds())} seconds"
        await Lyrical.change_presence(activity=nextcord.Activity(type=nextcord.ActivityType.listening, name=presence))
        await asyncio.sleep(1)  # Update presence every minute

cogs_folder = os.path.join("source", "cogs")
for f in os.listdir(cogs_folder):
	if f.endswith(".py"):
		Lyrical.load_extension("cogs." + f[:-3])
		
Lyrical.run(token)