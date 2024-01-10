import os

tokens_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'tokens')

class config: 
    # Read token.key file
    def get_token():
        token_file_path = os.path.join(tokens_dir, "token.key")
        with open(token_file_path, "r") as token_file:
            return token_file.read().strip()

    # Read guild.id file
    def get_guild_id():
        guild_file_path = os.path.join(tokens_dir, "guild.id")
        with open(guild_file_path, "r") as guild_file:
            return guild_file.read().strip()
