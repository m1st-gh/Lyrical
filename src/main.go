package main

import (
	"fmt"
	"os"
	"os/signal"
	"time"
)

func main() {
	start := time.Now()

	SetLoggerOutput("lyrical")
	bot := NewBot()
	bot.loadCommands(commands)
	bot.Session.AddHandler(bot.onVoiceServerUpdate)
	bot.Session.AddHandler(bot.onVoiceStateUpdate)

	defer bot.Session.Close()
	
	shutdownSignal := make(chan os.Signal, 1)
	signal.Notify(shutdownSignal, os.Interrupt)
	Info("Lyrical up in: %vms", time.Since(start).Milliseconds())
	SessionMessage := bot.Config.STATUS
	go func() {
		for {
			uptime := time.Since(start).Round(time.Second)
			hours := int(uptime.Hours())
			minutes := int(uptime.Minutes()) % 60
			formattedUptime := fmt.Sprintf("%02d:%02d", hours, minutes)
			bot.Session.UpdateCustomStatus(fmt.Sprintf("Uptime: %v, %v", formattedUptime, SessionMessage))
			time.Sleep(1 * time.Minute)
		}
	}()
	<-shutdownSignal
	Info("Gracefully shutting down.")
}
