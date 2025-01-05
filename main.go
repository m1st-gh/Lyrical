package main

import (
	"os"
	"os/signal"
)

func main() {

	SetLoggerOutput("lyrical.log")
	bot := NewBot()
	bot.loadCommands(commands)
	bot.Session.AddHandler(bot.onVoiceServerUpdate)
	bot.Session.AddHandler(bot.onVoiceStateUpdate)

	defer bot.Session.Close()

	shutdownSignal := make(chan os.Signal, 1)
	signal.Notify(shutdownSignal, os.Interrupt)
	Info("Press Ctrl+C to exit")
	<-shutdownSignal
	Info("Gracefully shutting down.")
}
