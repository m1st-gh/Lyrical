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
		Name:        "dequeue",
		Description: "Removes a track from the queue",
		Options: []*discordgo.ApplicationCommandOption{
			{
				Type:        discordgo.ApplicationCommandOptionInteger,
				Name:        "index",
				Description: "The index of the track to remove, if not provided the last track is removed",
				Required:    false,
			},
		},
	},
}
