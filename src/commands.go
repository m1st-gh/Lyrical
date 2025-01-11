package main

import (
	"github.com/bwmarrin/discordgo"
)

var commands = []*discordgo.ApplicationCommand{
	{
		Name:        "ping",
		Description: "Returns pong",
	},
	{
		Name:        "play",
		Description: "Plays a track",
		Options: []*discordgo.ApplicationCommandOption{
			{
				Type:        discordgo.ApplicationCommandOptionString,
				Name:        "track",
				Description: "The track to be played or searched",
				Required:    true,
			},
		},
	},
	{
		Name: 	  "queue",
		Description: "Shows the current queue",
	},
}
