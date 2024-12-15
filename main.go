package main

import (
	"context"
	"os"
	"os/signal"

	"github.com/bwmarrin/discordgo"
	"github.com/disgoorg/disgolink/v3/disgolink"
	"github.com/disgoorg/snowflake/v2"
	"github.com/joho/godotenv"
)

type Config struct {
	DISCORD_API_KEY string
	GUILD           string
	LINK_NAME       string
	LINK_ADDRESS    string
	LINK_PASSWORD   string
}

type DiscordCommand struct {
	Command *discordgo.ApplicationCommand
	Handler func(*discordgo.Session, *discordgo.InteractionCreate)
}

func init()

func initConfig() Config {
	err := godotenv.Load()
	if err != nil {
		Fatal("Error Loading .env %v", err)
	}

	DISCORD_API_KEY := os.Getenv("DISCORD_API_KEY")
	GUILD := os.Getenv("TEST_GUILD")
	LINK_NAME := os.Getenv("LINK_NAME")
	LINK_ADDRESS := os.Getenv("LINK_ADDRESS")
	LINK_PASSWORD := os.Getenv("LINK_PASSWORD")
	config := Config{
		DISCORD_API_KEY: DISCORD_API_KEY,
		GUILD:           GUILD,
		LINK_NAME:       LINK_NAME,
		LINK_ADDRESS:    LINK_ADDRESS,
		LINK_PASSWORD:   LINK_PASSWORD,
	}
	return config
}

func initBot(config Config) *discordgo.Session {
	session, err := discordgo.New("Bot " + config.DISCORD_API_KEY)
	if err != nil {
		Fatal("Error opening connection: %v", err)
	}

	session.AddHandler(func(s *discordgo.Session, r *discordgo.Ready) {
		Info("Logged in as: %v", s.State.User.Username)
	})
	err = session.Open()
	if err != nil {
		Fatal("Session failed to open")
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
			Fatal("Cannot create \"%v\" command with error: %v", command.Command.Name, err)
		}
		registeredCommands[i] = createdCommand
	}
}

func initLink(session *discordgo.Session, config Config) disgolink.Node {
	link := disgolink.New(snowflake.MustParse(session.State.User.ID))
	node, err := link.AddNode(context.TODO(), disgolink.NodeConfig{
		Name:      config.LINK_NAME,
		Address:   config.LINK_ADDRESS,
		Password:  config.LINK_PASSWORD,
		Secure:    false,
		SessionID: "",
	})
	if err != nil {
		Warn("Error initializing link: %v", err)
	}
	return node
}

func main() {

	config := initConfig()
	session := initBot(config)
	node := initLink(session, config)

	var commands = []DiscordCommand{
		{
			Command: &discordgo.ApplicationCommand{
				Name:        "ping",
				Description: "pong",
			},
			Handler: ping,
		},
	}

	loadCommands(config, session, commands)

	defer session.Close()

	shutdownSignal := make(chan os.Signal, 1)
	signal.Notify(shutdownSignal, os.Interrupt)
	Info("Press Ctrl+C to exit")
	<-shutdownSignal

	Info("Gracefully shutting down.")
}
