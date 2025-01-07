# Lyrical

Lyrical is a Discord bot written in Go that allows users to play music in their Discord servers. It leverages the DiscordGo library for interacting with the Discord API and Disgolink for handling music playback.

## Features

- **Play Music**: Play tracks from various sources.
- **Queue Management**: Add, remove, and view tracks in the queue.
- **Playback Controls**: Pause, resume, skip, and stop tracks.
- **Shuffle and Repeat**: Shuffle the queue and toggle repeat mode.
- **Custom Status**: Display bot's uptime and custom status.

## Getting Started

### Prerequisites

- Go 1.23 or later
- Docker (optional, for containerized deployment)

### Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/m1st-gh/Lyrical.git
    cd Lyrical
    ```

2. Install dependencies:
    ```sh
    go mod download
    ```

3. Create a `.env` file and fill in the required environment variables:
    ```env
    DISCORD_API_KEY=your_discord_api_key
    TEST_GUILD=your_test_guild_id
    LINK_NAME=your_link_name
    LINK_ADDRESS=your_link_address
    LINK_PASSWORD=your_link_password
    ```

### Running the Bot

1. Build and run the bot:
    ```sh
    go build -o Lyrical
    ./Lyrical
    ```

2. Alternatively, you can use Docker:
    ```sh
    docker build -t lyrical .
    docker run --env-file .env lyrical
    ```

## Usage

- **/ping**: Check the bot's latency.
- **/play [track]**: Play a track or search for a track.
- **/queue**: Display the current queue.
- **/pause**: Pause the current track.
- **/resume**: Resume the paused track.
- **/skip**: Skip to the next track.
- **/stop**: Stop playback and clear the queue.
- **/shuffle**: Shuffle the current queue.
- **/repeat**: Toggle repeat mode.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.

## License

**todo**

## Acknowledgements

- [DiscordGo](https://github.com/bwmarrin/discordgo)
- [Disgolink](https://github.com/disgoorg/disgolink)
