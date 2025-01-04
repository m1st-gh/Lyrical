package main

import (
	"context"

	"github.com/disgoorg/disgolink/v3/disgolink"
	"github.com/disgoorg/disgolink/v3/lavalink"
)

func (b *Bot) onPlayerPause(player disgolink.Player, event lavalink.PlayerPauseEvent) {
	Info("onPlayerPause: %v", event)
}

func (b *Bot) onPlayerResume(player disgolink.Player, event lavalink.PlayerResumeEvent) {
	Info("onPlayerResume: %v", event)
}

func (b *Bot) onTrackStart(player disgolink.Player, event lavalink.TrackStartEvent) {
	// Info("onTrackStart: %v", event)
}

func (b *Bot) onTrackEnd(player disgolink.Player, event lavalink.TrackEndEvent) {
	Info("onTrackEnd: %v", event.Track.Info.Title)

	if !event.Reason.MayStartNext() {
		return
	}

	var err error
	var nextTrack lavalink.Track

	queue := b.Queue[event.GuildID().String()]
	nextTrack, err = queue.Next()

	if err != nil {
		Info("Queue next failed: %v", err)
		return
	}
	err = player.Update(context.TODO(), lavalink.WithTrack(nextTrack))
	if err != nil {
		Error("Failed to play next track: %v", err)
	}
}

func (b *Bot) onTrackException(player disgolink.Player, event lavalink.TrackExceptionEvent) {
	Info("onTrackException: %v", event)
}

func (b *Bot) onTrackStuck(player disgolink.Player, event lavalink.TrackStuckEvent) {
	Info("onTrackStuck: %v", event)
}

func (b *Bot) onWebSocketClosed(player disgolink.Player, event lavalink.WebSocketClosedEvent) {
	Info("onWebSocketClosed: %v", event)
}
