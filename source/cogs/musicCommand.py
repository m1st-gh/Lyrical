#TODO:
###############################################
#   - Auto add playlist support
#   - Add multipul songs at once
###############################################
from nextcord.ext import commands, menus, tasks
import nextcord
import os
import sys
lib = os.path.join(os.path.dirname(__file__), 'lib')
sys.path.append(lib)
from lib.config import config as tokens
from lib.songQueue import SongQueue

DEFAULT_GUILD_ID = int(tokens.get_guild_id())

songs = SongQueue()
stream = None
musicMenuRunning = False


@tasks.loop(seconds=1)
async def playLoop(interaction: nextcord.Interaction, quels: SongQueue(), msg: nextcord.Message):
    global stream
    if not stream.is_playing():
        quels.dequeue()
        stream.play(nextcord.FFmpegPCMAudio(quels.front()['url']))
        await msg.edit(embed=embed)
        return

class Music(commands.Cog):

    def __init__(self, bot):
        self.bot = bot
    @nextcord.slash_command(name='play', description='Play a song', guild_ids=[DEFAULT_GUILD_ID])
        
    async def play(self, interaction: nextcord.Interaction, url: str):
        global stream
        await interaction.response.defer() 
        #if in server move to, else check for joined, reject if not.
        if (interaction.guild.voice_client is not None and interaction.user.voice.channel != interaction.guild.voice_client.channel):
            stream.move_to(interaction.user.voice.channel)
        elif interaction.user.voice is None:
            return await interaction.followup.send('You are not in a voice channel', ephemeral=True, delete_after=5)
        elif stream is None:
            stream = await interaction.user.voice.channel.connect()
        
        #Check if stream is running, if song is not downloaded
        if songs.push(url):
            if stream.is_playing():
                await interaction.followup.send('Song added to queue', ephemeral=True, delete_after=5)
        else:
            return await interaction.followup.send('Song not found', ephemeral=True, delete_after=5)
        if stream.is_playing() is False:
            songs.setCurrentSong(songs.peek(-1))
            stream.play(nextcord.FFmpegPCMAudio(songs.getCurrentSong()['url']))
            if musicMenuRunning is False:
                await Music.musicMenu().start(interaction=interaction)
        

    def createEmbed():
        global songs
        embed = nextcord.Embed(title=songs.getCurrentSong()['title'][:128], url=songs.getCurrentSong()['webpage_url'])
        embed.set_image(url=songs.getCurrentSong()['thumbnail'])
        return embed

    class musicMenu(menus.ButtonMenu):
        def __init__(self):
            super().__init__(disable_buttons_after=True)

        async def send_initial_message(self, ctx, channel):
            global stream, songs, musicMenuRunning
            musicMenuRunning = True
            await self.interaction.followup.send(embed=Music.createEmbed(), view=self)
            return await self.interaction.original_message()
        
        @nextcord.ui.button(emoji='⏮️', label='Back')
        async def backSong(self, button, interaction):
            global stream, songs
            if stream.is_playing() and stream.timestamp > 5:
                stream.stop()
                stream.play(nextcord.FFmpegPCMAudio(songs.current['url']))
            elif stream.is_playing() is False or stream.timestamp <= 5:
                stream.play(nextcord.FFmpegPCMAudio(songs.current['url']))
                songs.setCurrentSong(songs.peek(songs.currentIndex() + 1))
            
                

        @nextcord.ui.button(emoji='⏯️', label='Pause/Resume')
        async def pauseSong(self, button, interaction):
            global stream, songs
            if stream.is_playing():
                stream.pause()
            else:
                stream.resume()
        
        @nextcord.ui.button(emoji='⏭️', label='Skip')
        async def skipSong(self, button, interaction):
            global stream, songs
            stream.stop()
            songs.setCurrentSong(songs.peek(songs.currentIndex() - 1))
            await self.interaction.edit_original_message(embed=Music.createEmbed(), view=self)
            stream.play(nextcord.FFmpegPCMAudio(songs.getCurrentSong()['url']))
               
        @nextcord.ui.button(emoji='⏹️', label='Stop')
        async def on_stop(self, button, interaction):
            global stream, songs, musicMenuRunning
            stream.stop()
            await stream.disconnect(force=True)
            musicMenuRunning = False
            self.stop()


def setup(bot):
    bot.add_cog(Music(bot))
    
