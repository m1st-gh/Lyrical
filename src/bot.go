package main

import (
	"context"
	"os"

	"github.com/bwmarrin/discordgo"
	"github.com/disgoorg/disgolink/v3/disgolink"
	"github.com/disgoorg/disgolink/v3/lavalink"
	"github.com/disgoorg/snowflake/v2"
	"github.com/joho/godotenv"
)

type Config struct {
	DISCORD_API_KEY string
	GUILD           string
	LINK_NAME       string
	LINK_ADDRESS    string
	LINK_PASSWORD   string
	STATUS          string
}

type Bot struct {
	Config   Config
	Session  *discordgo.Session
	Link     disgolink.Client
	Node     disgolink.Node
	Commands []*discordgo.ApplicationCommand
	Handlers map[string]func(interaction *discordgo.InteractionCreate)
	State    map[string]*State
}

type State struct {
	Queue    *Queue[lavalink.Track]
	Player   *discordgo.Message
	IsRepeat bool
}

func NewBot() *Bot {
	bot := &Bot{}
	bot.initConfig()
	bot.initBot()
	bot.initLink()
	bot.initHandlers()
	return bot
}

func (b *Bot) initConfig() {
	if err := godotenv.Load(); err != nil {
		Fatal("Error loading env: %v", err)
	}

	b.Config = Config{
		DISCORD_API_KEY: os.Getenv("DISCORD_API_KEY"),
		GUILD:           os.Getenv("TEST_GUILD"),
		LINK_NAME:       os.Getenv("LINK_NAME"),
		LINK_ADDRESS:    os.Getenv("LINK_ADDRESS"),
		LINK_PASSWORD:   os.Getenv("LINK_PASSWORD"),
		STATUS:          os.Getenv("STATUS"),
	}
}

func (b *Bot) initBot() {
	session, err := discordgo.New("Bot " + b.Config.DISCORD_API_KEY)
	if err != nil {
		Fatal("Error opening connection: %v", err)
	}

	session.AddHandler(func(s *discordgo.Session, r *discordgo.Ready) {
		Info("Logged in as: %v", s.State.User.Username)
	})

	if err := session.Open(); err != nil {
		Fatal("Session failed to open")
	}
	b.State = make(map[string]*State)
	b.Session = session
}

func (b *Bot) initLink() {
	link := disgolink.New(snowflake.MustParse(b.Session.State.User.ID),
		disgolink.WithListenerFunc(b.onPlayerPause),
		disgolink.WithListenerFunc(b.onPlayerResume),
		disgolink.WithListenerFunc(b.onTrackStart),
		disgolink.WithListenerFunc(b.onTrackEnd),
		disgolink.WithListenerFunc(b.onTrackException),
		disgolink.WithListenerFunc(b.onTrackStuck),
		disgolink.WithListenerFunc(b.onWebSocketClosed))
	node, err := link.AddNode(context.TODO(), disgolink.NodeConfig{
		Name:      b.Config.LINK_NAME,
		Address:   b.Config.LINK_ADDRESS,
		Password:  b.Config.LINK_PASSWORD,
		Secure:    false,
		SessionID: "",
	})
	if err != nil {
		Warn("Error initializing link: %v", err)
	}
	b.Node = node
	b.Link = link
}

func (b *Bot) loadCommands(commands []*discordgo.ApplicationCommand) {
	registeredCommands := make([]*discordgo.ApplicationCommand, len(commands))
	for i, command := range commands {
		createdCommand, err := b.Session.ApplicationCommandCreate(b.Session.State.User.ID, b.Config.GUILD, command)
		if err != nil {
			Fatal("Cannot create \"%v\" command with error: %v", command.Name, err)
		}
		registeredCommands[i] = createdCommand
	}

}
