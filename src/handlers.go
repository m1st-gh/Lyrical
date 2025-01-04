package main

import (
	"context"
	"fmt"
	"regexp"
	"time"

	"github.com/bwmarrin/discordgo"
	"github.com/disgoorg/disgolink/v3/disgolink"
	"github.com/disgoorg/disgolink/v3/lavalink"
	"github.com/disgoorg/snowflake/v2"
)

var (
	urlPattern    = regexp.MustCompile(`^https?://[-a-zA-Z0-9+&@#/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_|]?`)
	searchPattern = regexp.MustCompile(`^(.{2})search:(.+)`)
)

func (b *Bot) ping(interaction *discordgo.InteractionCreate) {
	start, _ := discordgo.SnowflakeTimestamp(interaction.ID)
	Info("Ping called by user: %v", interaction.Member.User.Username)
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseChannelMessageWithSource,
		Data: &discordgo.InteractionResponseData{
			Embeds: []*discordgo.MessageEmbed{
				{
					Title:       "Pong!",
					Description: fmt.Sprintf("Latency: %v", time.Since(start).Abs().Round(time.Millisecond).String()),
				},
			},
		},
	})
}

func verifyUri(uri string) (string, error) {
	if uri == "" {
		return "", fmt.Errorf("empty input")
	}
	if !urlPattern.MatchString(uri) && !searchPattern.MatchString(uri) {
		return lavalink.SearchTypeYouTube.Apply(uri), nil
	}
	return uri, nil
}

func (b *Bot) embedQueue(interaction *discordgo.InteractionCreate) *discordgo.MessageEmbed {
	var discription string = ""
	queue := b.Queue[interaction.GuildID]
	for i, track := range queue.Items() {
		discription += fmt.Sprintf("%v: [%v](%v)\n", i+1, track.Info.Title, track.Info.URI)
	}
	embed := &discordgo.MessageEmbed{
		Title:       "Lyrical",
		Description: discription,
	}
	return embed
}

func (b *Bot) embedNowPlaying(interaction *discordgo.InteractionCreate) (*discordgo.MessageEmbed, []discordgo.MessageComponent) {
	buttons := []discordgo.MessageComponent{
		discordgo.ActionsRow{
			Components: []discordgo.MessageComponent{
				discordgo.Button{
					Label:    "⏮️",
					Style:    discordgo.SecondaryButton,
					CustomID: "b_backward",
				},
				discordgo.Button{
					Label:    "⏯️",
					Style:    discordgo.SecondaryButton,
					CustomID: "b_resumePause",
				},
				discordgo.Button{
					Label:    "⏭️",
					Style:    discordgo.SecondaryButton,
					CustomID: "b_forward",
				},
			},
		},
	}

	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	track := player.Track()
	if track == nil {
		return nil, buttons
	}
	embed := &discordgo.MessageEmbed{
		Title:       "Now Playing",
		Image:       &discordgo.MessageEmbedImage{URL: fmt.Sprintf("%v?width=854&height=480", *track.Info.ArtworkURL)},
		Description: fmt.Sprintf("[%v](%v)", track.Info.Title, *track.Info.URI),
	}
	return embed, buttons
}

func (b *Bot) resumePause(interaction *discordgo.InteractionCreate) {
	Info("Resume/Pause called by user: %v", interaction.Member.User.Username)
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		return
	}
	if player.Paused() {
		player.Update(context.TODO(), lavalink.WithPaused(false))
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "Resumed",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	} else {
		player.Update(context.TODO(), lavalink.WithPaused(true))
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "Paused",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}

}

func (b *Bot) forward(interaction *discordgo.InteractionCreate) {
	Info("Forward called by user: %v", interaction.Member.User.Username)
	queue := b.Queue[interaction.GuildID]
	if queue == nil {
		return
	}
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		return
	}
	next, _ := queue.Next()
	player.Update(context.TODO(), lavalink.WithTrack(next))
}

func (b *Bot) backward(interaction *discordgo.InteractionCreate) {
	Info("Backward called by user: %v", interaction.Member.User.Username)
	queue := b.Queue[interaction.GuildID]
	if queue == nil {
		return
	}
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		return
	}
	previous, _ := queue.Prev()
	player.Update(context.TODO(), lavalink.WithTrack(previous))
}

func (b *Bot) play(interaction *discordgo.InteractionCreate) {

	voiceState, err := b.Session.State.VoiceState(interaction.GuildID, interaction.Member.User.ID)
	if err != nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "Please join a voice channel!",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}
	if b.Queue[interaction.GuildID] == nil {
		b.Queue[interaction.GuildID] = NewQueue[lavalink.Track]()
	}

	queue := b.Queue[interaction.GuildID]
	trackname, err := verifyUri(interaction.ApplicationCommandData().Options[0].StringValue())
	if err != nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "Please specify a uri!",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	var trackToPlay *lavalink.Track
	b.Link.BestNode().LoadTracksHandler(ctx, trackname, disgolink.NewResultHandler(
		func(track lavalink.Track) {
			if player.Track() == nil {
				trackToPlay = &track
				queue.Enqueue(track)
			} else {
				queue.Enqueue(track)
			}
		},
		func(playlist lavalink.Playlist) {
			if player.Track() == nil {
				trackToPlay = &playlist.Tracks[0]
				queue.Enqueue(playlist.Tracks...)
			} else {
				queue.Enqueue(playlist.Tracks...)
			}
		},
		func(tracks []lavalink.Track) {
			if player.Track() == nil {
				trackToPlay = &tracks[0]
				queue.Enqueue(tracks[0])
			} else {
				queue.Enqueue(tracks[0])
			}
		},
		func() {},
		func(err error) {},
	))

	if trackToPlay == nil {
		return
	}
	if err := b.Session.ChannelVoiceJoinManual(interaction.GuildID, voiceState.ChannelID, false, true); err != nil {
		return
	}

	player.Update(context.TODO(), lavalink.WithTrack(*trackToPlay))
	embed, buttons := b.embedNowPlaying(interaction)

	if b.Player[interaction.GuildID] == nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Embeds:     []*discordgo.MessageEmbed{embed},
				Components: buttons,
				Title:      "Now Playing",
			},
		})
		b.Player[interaction.GuildID], err = b.Session.InteractionResponse(interaction.Interaction)
		if err != nil {
			Error("Error sending message: %v", err)
		}
		Info("Message ID: %v", b.Player[interaction.GuildID].ID)
	}

}

func (b *Bot) onVoiceStateUpdate(session *discordgo.Session, event *discordgo.VoiceStateUpdate) {
	if event.UserID != session.State.User.ID {
		return
	}

	var channelID *snowflake.ID
	if event.ChannelID != "" {
		id := snowflake.MustParse(event.ChannelID)
		channelID = &id
	}
	b.Link.OnVoiceStateUpdate(context.TODO(), snowflake.MustParse(event.GuildID), channelID, event.SessionID)
	if event.ChannelID == "" {
		b.Queue[event.GuildID].Clear()
	}
}

func (b *Bot) onVoiceServerUpdate(session *discordgo.Session, event *discordgo.VoiceServerUpdate) {
	b.Link.OnVoiceServerUpdate(context.TODO(), snowflake.MustParse(event.GuildID), event.Token, event.Endpoint)
}

func (b *Bot) initHandlers() map[string]func(interaction *discordgo.InteractionCreate) {
	handlers := map[string]func(interaction *discordgo.InteractionCreate){
		"ping":          b.ping,
		"play":          b.play,
		"b_forward":     b.forward,
		"b_backward":    b.backward,
		"b_resumePause": b.resumePause,
	}

	b.Handlers = handlers

	b.Session.AddHandler(func(session *discordgo.Session, interaction *discordgo.InteractionCreate) {
		switch interaction.Type {
		case discordgo.InteractionApplicationCommand:
			if handler, exists := b.Handlers[interaction.ApplicationCommandData().Name]; exists {
				handler(interaction)
			}
		case discordgo.InteractionMessageComponent:
			// Handle component interactions (buttons, select menus, etc.)
			if handler, exists := b.Handlers[interaction.MessageComponentData().CustomID]; exists {
				handler(interaction)
			}
		}
	})

	return handlers
}
