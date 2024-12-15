package main

import "github.com/bwmarrin/discordgo"

func ping(session *discordgo.Session, interaction *discordgo.InteractionCreate) {
	Info("Ping called by user: %v", interaction.Member.User.Username)
	session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseChannelMessageWithSource,
		Data: &discordgo.InteractionResponseData{
			Content: "Pong",
		},
	})

}
