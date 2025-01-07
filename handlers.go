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
	queue := b.State[interaction.GuildID].Queue
	if queue.IsEmpty() {
		embed := &discordgo.MessageEmbed{
			Title:       "**Current Queue**",
			Description: "Queue is empty",
		}
		return embed
	}
	for i, track := range queue.Items() {
		title := track.Info.Title
		if len(title) > 40 {
			title = title[:40] + "..."
		}
		if i == queue.Current() {
			discription += fmt.Sprintf("**%v:** [%v](%v) (Currently Playing)\n", i+1, title, *track.Info.URI)
		} else {
			discription += fmt.Sprintf("**%v:** [%v](%v)\n", i+1, title, *track.Info.URI)
		}
	}

	embed := &discordgo.MessageEmbed{
		Title:       "**Current Queue**",
		Description: discription,
	}
	return embed
}

func (b *Bot) queue(interaction *discordgo.InteractionCreate) {
	Info("Queue called by user: %v", interaction.Member.User.Username)
	embed := b.embedQueue(interaction)
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseChannelMessageWithSource,
		Data: &discordgo.InteractionResponseData{
			Embeds: []*discordgo.MessageEmbed{embed},
		},
	})
}

func (b *Bot) embedNowPlaying(guildID string) (*discordgo.MessageEmbed, []discordgo.MessageComponent) {
	buttons := []discordgo.MessageComponent{
		discordgo.ActionsRow{
			Components: []discordgo.MessageComponent{
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325505582828097668",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_queue",
				},
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325505582081376341",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_backward",
				},
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325505587508936874",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_resumePause",
				},
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325505586481074297",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_forward",
				},
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325505588540735581",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_stop",
				},
			},
		},
		discordgo.ActionsRow{
			Components: []discordgo.MessageComponent{
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325534933011402773",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_shuffle",
				},
				discordgo.Button{
					Emoji: &discordgo.ComponentEmoji{
						ID: "1325534932025475142",
					},
					Style:    discordgo.SecondaryButton,
					CustomID: "b_repeat",
				},
			},
		},
	}

	player := b.Link.Player(snowflake.MustParse(guildID))
	track := player.Track()
	if track == nil {
		return nil, buttons
	}
	embed := &discordgo.MessageEmbed{
		Title:       "Now Playing",
		Image:       &discordgo.MessageEmbedImage{URL: fmt.Sprintf("%v", *track.Info.ArtworkURL)},
		Description: fmt.Sprintf("[%v](%v)", track.Info.Title, *track.Info.URI),
	}
	return embed, buttons
}
func (b *Bot) bShuffle(interaction *discordgo.InteractionCreate) {
	Info("Shuffle called by user: %v", interaction.Member.User.Username)
	queue := b.State[interaction.GuildID].Queue
	if queue == nil {
		return
	}
	queue.Shuffle()
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseDeferredMessageUpdate,
	})
}

func (b *Bot) bStop(interaction *discordgo.InteractionCreate) {
	Info("Stop called by user: %v", interaction.Member.User.Username)
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	b.Session.ChannelMessageDelete(interaction.ChannelID, b.State[interaction.GuildID].Player.ID)
	b.State[interaction.GuildID].Player = nil
	b.State[interaction.GuildID].Queue.Clear()
	player.Update(context.TODO(), lavalink.WithNullTrack())
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseChannelMessageWithSource,
		Data: &discordgo.InteractionResponseData{
			Content: "Stopped",
			Flags:   discordgo.MessageFlagsEphemeral,
		},
	})
}

func (b *Bot) bQueue(interaction *discordgo.InteractionCreate) {
	Info("Queue called by user: %v", interaction.Member.User.Username)
	embed := b.embedQueue(interaction)
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseChannelMessageWithSource,
		Data: &discordgo.InteractionResponseData{
			Embeds: []*discordgo.MessageEmbed{embed},
			Flags:  discordgo.MessageFlagsEphemeral,
		},
	})
}

func (b *Bot) bResumePause(interaction *discordgo.InteractionCreate) {
	Info("Resume/Pause called by user: %v", interaction.Member.User.Username)
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		return
	}
	if player.Paused() {
		player.Update(context.TODO(), lavalink.WithPaused(false))
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseDeferredMessageUpdate,
		})
		return
	} else {
		player.Update(context.TODO(), lavalink.WithPaused(true))
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseDeferredMessageUpdate,
		})
		return
	}

}

func (b *Bot) bForward(interaction *discordgo.InteractionCreate) {
	Info("Forward called by user: %v", interaction.Member.User.Username)
	queue := b.State[interaction.GuildID].Queue
	if queue == nil {
		return
	}
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "No track playing to skip!",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}
	next, err := queue.Next()
	if err != nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "End of queue!",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}

	player.Update(context.TODO(), lavalink.WithTrack(next))
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseDeferredMessageUpdate,
	})
}

func (b *Bot) bBackward(interaction *discordgo.InteractionCreate) {
	Info("Backward called by user: %v", interaction.Member.User.Username)
	queue := b.State[interaction.GuildID].Queue
	if queue == nil {
		return
	}
	player := b.Link.Player(snowflake.MustParse(interaction.GuildID))
	if player.Track() == nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: "No track playing to skip!",
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}
	if player.State().Position.Milliseconds() < 5000 {
		prev, _ := queue.Prev()
		player.Update(context.TODO(), lavalink.WithTrack(prev))
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseDeferredMessageUpdate,
		})
		return
	}
	player.Update(context.TODO(), lavalink.WithPosition(0))
	b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
		Type: discordgo.InteractionResponseDeferredMessageUpdate,
	})
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
	if b.State[interaction.GuildID] == nil {
		b.State[interaction.GuildID] = &State{}
		b.State[interaction.GuildID].Queue = NewQueue[lavalink.Track]()
	}

	queue := b.State[interaction.GuildID].Queue
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
	var message string = ""
	b.Link.BestNode().LoadTracksHandler(ctx, trackname, disgolink.NewResultHandler(
		func(track lavalink.Track) {
			if player.Track() == nil {
				trackToPlay = &track
				if queue.size >= 1 {
					queue.Enqueue(track)
					queue.Next()
					message = fmt.Sprintf("Added to queue: [%v](%v)", track.Info.Title, *track.Info.URI)
				} else {
					queue.Enqueue(track)
				}
			} else {
				queue.Enqueue(track)
				message = fmt.Sprintf("Added to queue: [%v](%v)", track.Info.Title, *track.Info.URI)
			}
		},
		func(playlist lavalink.Playlist) {
			if player.Track() == nil {
				trackToPlay = &playlist.Tracks[0]
				if queue.size >= 1 {
					queue.Enqueue(playlist.Tracks[0])
					queue.Next()
					message = fmt.Sprintf("Added to queue: [%v](%v)", playlist.Tracks[0].Info.Title, *playlist.Tracks[0].Info.URI)
				} else {
					queue.Enqueue(playlist.Tracks[0])
				}
			} else {
				queue.Enqueue(playlist.Tracks...)
				message = fmt.Sprintf("Added playlist to queue: %v tracks", len(playlist.Tracks))
			}
		},
		func(tracks []lavalink.Track) {
			// ToDo: implement search result handler if requested.
			if player.Track() == nil {
				trackToPlay = &tracks[0]
				if queue.size >= 1 {
					queue.Enqueue(tracks[0])
					queue.Next()
					message = fmt.Sprintf("Added to queue: [%v](%v)", tracks[0].Info.Title, *tracks[0].Info.URI)
				} else {
					queue.Enqueue(tracks[0])
				}
			} else {
				queue.Enqueue(tracks[0])
				message = fmt.Sprintf("Added to queue: [%v](%v)", tracks[0].Info.Title, *tracks[0].Info.URI)
			}
		},
		func() {},
		func(err error) {},
	))

	if trackToPlay == nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: message,
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		return
	}
	if err := b.Session.ChannelVoiceJoinManual(interaction.GuildID, voiceState.ChannelID, false, true); err != nil {
		return
	}

	player.Update(context.TODO(), lavalink.WithTrack(*trackToPlay))
	embed, buttons := b.embedNowPlaying(interaction.GuildID)
	if b.State[interaction.GuildID].Player != nil {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Content: message,
				Flags:   discordgo.MessageFlagsEphemeral,
			},
		})
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseDeferredMessageUpdate,
		})
	} else {
		b.Session.InteractionRespond(interaction.Interaction, &discordgo.InteractionResponse{
			Type: discordgo.InteractionResponseChannelMessageWithSource,
			Data: &discordgo.InteractionResponseData{
				Embeds:     []*discordgo.MessageEmbed{embed},
				Components: buttons,
			},
		})
		b.State[interaction.GuildID].Player, err = b.Session.InteractionResponse(interaction.Interaction)
	}
	if err != nil {
		Error("%v", err)
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
		b.State[event.GuildID].Queue.Clear()
	}
}

func (b *Bot) onVoiceServerUpdate(session *discordgo.Session, event *discordgo.VoiceServerUpdate) {
	b.Link.OnVoiceServerUpdate(context.TODO(), snowflake.MustParse(event.GuildID), event.Token, event.Endpoint)
}

func (b *Bot) initHandlers() map[string]func(interaction *discordgo.InteractionCreate) {
	handlers := map[string]func(interaction *discordgo.InteractionCreate){
		"ping":          b.ping,
		"play":          b.play,
		"queue":         b.queue,
		"b_forward":     b.bForward,
		"b_backward":    b.bBackward,
		"b_resumePause": b.bResumePause,
		"b_queue":       b.bQueue,
		"b_stop":        b.bStop,
		"b_shuffle":     b.bShuffle,
	}

	b.Handlers = handlers

	b.Session.AddHandler(func(session *discordgo.Session, interaction *discordgo.InteractionCreate) {
		switch interaction.Type {
		case discordgo.InteractionApplicationCommand:
			if handler, exists := b.Handlers[interaction.ApplicationCommandData().Name]; exists {
				handler(interaction)
			}
		case discordgo.InteractionMessageComponent:
			if handler, exists := b.Handlers[interaction.MessageComponentData().CustomID]; exists {
				handler(interaction)
			}
		}
	})

	return handlers
}
