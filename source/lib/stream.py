import nextcord.ext 
import nextcord
import os

class stream():
    def __init__(self, bot, Interaction: nextcord.Interaction):
        self.bot = bot
        self.stream = None
        self.interaction = Interaction

    async def play(self, ):
