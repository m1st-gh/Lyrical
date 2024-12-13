package main

import (
  "log"
  "os"
  "os/signal"

  "github.com/bwmarrin/discordgo"
  "github.com/joho/godotenv"
)

type Config struct {
  DISCORD_API_KEY string
  GUILD           string
}

type DiscordCommand struct {
  Command *discordgo.ApplicationCommand
  Handler func(session *discordgo.Session, interaction *discordgo.InteractionCreate)
}

func init_config() Config {
  err := godotenv.Load()
  if err != nil {
    log.Fatalf("[FATAL] Error Loading .env %v", err)
  }

  DISCORD_API_KEY := os.Getenv("DISCORD_API_KEY")
  GUILD := os.Getenv("TEST_GUILD")
  config := Config{
    DISCORD_API_KEY: DISCORD_API_KEY,
    GUILD:           GUILD,
  }
  return config
}

func init_bot(config Config) *discordgo.Session {
  session, err := discordgo.New("Bot " + config.DISCORD_API_KEY)
  if err != nil {
    log.Fatal("[FATAL] Error opening connection:", err)
  }

  session.AddHandler(func(s *discordgo.Session, r *discordgo.Ready) {
    log.Printf("Logged in as: %v", s.State.User.Username)
  })

  if err := session.Open(); err != nil {
    log.Fatal("[FATAL] Session Failed to Open")
  }
  return session
}

func loadCommands(botConfig Config, session *discordgo.Session, discordCommands []DiscordCommand) {
  session.AddHandler(func(session *discordgo.Session, interactionCreate *discordgo.InteractionCreate) {
    for _, command := range discordCommands {
      if interactionCreate.ApplicationCommandData().Name == command.Command.Name {
        command.Handler(session, interactionCreate)
        break
      }
    }
  })
  registeredCommands := make([]*discordgo.ApplicationCommand, len(discordCommands))
  for i, command := range discordCommands {
    createdCommand, err := session.ApplicationCommandCreate(session.State.User.ID, botConfig.GUILD, command.Command)
    if err != nil {
      log.Panicf("[PANIC] Cannot create '%v' command: %v", command.Command.Name, err)
    }
    registeredCommands[i] = createdCommand
  }
}

func main() {
  var session *discordgo.Session
  var botConfig Config

  var commands = []DiscordCommand{
    {
      Command: &discordgo.ApplicationCommand{
        Name:        "ping",
        Description: "pong",
      },
      Handler: func(session *discordgo.Session, interaction *discordgo.InteractionCreate) {
        session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
          Type: discordgo.InteractionResponseChannelMessageWithSource,
          Data: &discordgo.InteractionResponseData{
            Content: "Pong",
          },
        })
      },
    },
  }

  botConfig = init_config()
  session = init_bot(botConfig)

  loadCommands(botConfig, session, commands)

  defer session.Close()

  shutdownSignal := make(chan os.Signal, 1)
  signal.Notify(shutdownSignal, os.Interrupt)
  log.Println("Press Ctrl+C to exit")
  <-shutdownSignal

  log.Println("Gracefully shutting down.")
}
